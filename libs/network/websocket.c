#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>

#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <openssl/ssl.h>
#include <openssl/sha.h>
#include <openssl/err.h>

#include <network.h>
#include <utils.h>

static void create_epoll(Websocket *websocket);
static void register_events(int epoll_fd, Websocket *websocket, uint32_t event_flags);
static void unregister_events(int epoll_fd, Websocket *websocket, uint32_t event_flags);
static void handle_events(Websocket *websocket, int epoll_fd, struct epoll_event *events, size_t num_events);

static void switch_protocols(Websocket *websocket);
static void check_response(Websocket *websocket, const char *response, const char *key);

static void initialize_websocket(Websocket *websocket, const char *url);



static void create_epoll(Websocket *websocket) {
	websocket->epollfd = epoll_create1(0);

	if (websocket->epollfd == -1) {
		throw_network("epoll_create1()", false);
	}
}

static void register_events(int epoll_fd, Websocket *websocket, uint32_t event_flags) {
	struct epoll_event event;
	event.data.fd = websocket->sockfd;
	event.events = event_flags;

	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, websocket->sockfd, &event) == -1) {
		throw_network("epoll_ctl()", !!websocket->ssl);
	}
}

static void unregister_events(int epoll_fd, Websocket *websocket, uint32_t event_flags) {
	struct epoll_event event;
	event.data.fd = websocket->sockfd;
	event.events = event_flags;

	if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, websocket->sockfd, &event) == -1) {
		throw_network("epoll_ctl()", !!websocket->ssl);
	}
}

static void handle_events(Websocket *websocket, int epoll_fd, struct epoll_event *events, size_t num_events) {
	for (int i = 0; i < num_events; ++i) {
		uint32_t event = events[i].events;

		if (events[i].data.fd == websocket->sockfd) {
			if (event & EPOLLERR || event & EPOLLHUP) {
				throw_network("socket error or hang up", false);
			} else if (event & EPOLLIN) {
				if ((i + 1) == num_events) {
					if (websocket->key) {
						char buffer[4096] = {0};
						_read(websocket->ssl, websocket->sockfd, buffer, 4095);

						if (strncmp(buffer + 9, "101", 3) == 0) {
							check_response(websocket, buffer, websocket->key);
							websocket->key = NULL;

							if (websocket->methods.onstart) {
								websocket->methods.onstart();
							}
						} else {
							throw_network("invalid http status code", !!websocket->ssl);
						}
					} else {
						WebsocketFrame frame;
						unsigned char buffer[9] = {0};

						_read(websocket->ssl, websocket->sockfd, (char *) buffer, 2);

						frame.fin = ((buffer[0] >> 7) & 0x1);
						frame.rsv[0] = ((buffer[0] >> 6) & 0x1);
						frame.rsv[1] = ((buffer[0] >> 5) & 0x1);
						frame.rsv[2] = ((buffer[0] >> 4) & 0x1);
						frame.opcode = (buffer[0] & 0xF);

						frame.mask = ((buffer[1] >> 7) & 0x1);
						frame.payload_length = (buffer[1] & 0x7F);

						if (frame.payload_length == 126) {
							_read(websocket->ssl, websocket->sockfd, (char *) buffer, 2);
							frame.payload_length = combine_bytes(buffer, 2);
						} else if (frame.payload_length == 127) {
							_read(websocket->ssl, websocket->sockfd, (char *) buffer, 8);
							frame.payload_length = combine_bytes(buffer, 8);
						}

						if (frame.mask == 0x1) {
							_read(websocket->ssl, websocket->sockfd, (char *) buffer, 4);
						}

						frame.payload = allocate(NULL, -1, frame.payload_length + 1, sizeof(char));

						switch (frame.opcode) {
							case 0x1: {
								size_t received = 0;

								while (received != frame.payload_length) {
									size_t diff = (frame.payload_length - received);

									if (diff > 512) {
										received += _read(websocket->ssl, websocket->sockfd, frame.payload + received, 512);
									} else {
										received += _read(websocket->ssl, websocket->sockfd, frame.payload + received, diff);
									}
								}

								if (websocket->methods.onmessage) {
									websocket->methods.onmessage(frame);
								}

								break;
							}

							case 0x8: {
								_read(websocket->ssl, websocket->sockfd, frame.payload, frame.payload_length);

								const short code = combine_bytes((unsigned char *) frame.payload, 2);
								const char *reason = (frame.payload + 2);
								close_websocket(websocket, code, (reason[0] == 0) ? NULL : reason);
								break;
							}
						}

						free(frame.payload);
					}
				}
			} else if (event & EPOLLOUT) {
				if (!websocket->connected && !websocket->closed) {
					switch_protocols(websocket);
				} else if (websocket->tbs_size != 0) {
					WebsocketTBS tbs = websocket->tbs[0];

					_write(websocket->ssl, websocket->sockfd, tbs.data, tbs.size);

					for (size_t i = 0; i < websocket->tbs_size; ++i) {
						if ((i + 1) != websocket->tbs_size) {
							websocket->tbs[i].data = allocate(websocket->tbs[i].data, -1, websocket->tbs[i + 1].size + 1, sizeof(char));
							websocket->tbs[i].size = websocket->tbs[i + 1].size;
							strncpy(websocket->tbs[i].data, websocket->tbs[i + 1].data, websocket->tbs[i + 1].size);
						}
					}

					free(websocket->tbs[websocket->tbs_size - 1].data);
					websocket->tbs = allocate(websocket->tbs, -1, websocket->tbs_size - 1, sizeof(WebsocketTBS));
					--websocket->tbs_size;
				}
			}
		}
	}
}

static void check_response(Websocket *websocket, const char *response, const char *key) {
	Split splitter = split((char *) response, "\r\n");

	for (int i = 0; i < splitter.size; ++i) {
		if (strstr(splitter.data[i], ":") != NULL) {
			Split line_splitter = split(splitter.data[i], ":");
			char header_name[128];
			strtolower(header_name, line_splitter.data[0]);

			if (strcmp(header_name, "sec-websocket-accept") == 0) {
				const char *value = ltrim(line_splitter.data[1]);

				const char* websocket_guid = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
				size_t final_length = strlen(key) + strlen(websocket_guid);
				char *final = allocate(NULL, -1, final_length + 1, sizeof(char));
				strcat(final, key);
				strcat(final, websocket_guid);

				unsigned char *sha1_hash = allocate(NULL, 0, SHA_DIGEST_LENGTH + 1, sizeof(char));
				SHA1((unsigned char *) final, final_length, sha1_hash);
				char *result = base64_encode((char *) sha1_hash, SHA_DIGEST_LENGTH);

				free((char *) key);
				free(sha1_hash);
				free(final);

				if (strcmp(result, value) != 0) {
					free(result);
					split_free(&line_splitter);
					split_free(&splitter);
					close_websocket(websocket, -1, NULL);

					throw_network("switch_protocols(): unmatched websocket keys", false);
				}

				free(result);

				split_free(&line_splitter);
				break;
			}

			split_free(&line_splitter);
		}
	}

	split_free(&splitter);
}

static void switch_protocols(Websocket *websocket) {
	unsigned char key_data[17] = {0};

	for (int i = 0; i < 16; ++i) {
		key_data[i] = ((rand() % 255) + 1);
	}

	websocket->key = base64_encode((char *) key_data, 16);

	char *request_message = allocate(NULL, -1, 512, sizeof(char));

	sprintf(request_message,
		"GET %s HTTP/1.1\r\n"
		"Host: %s:%d\r\n"
		"Upgrade: websocket\r\n"
		"Connection: Upgrade\r\n"
		"Sec-WebSocket-Key: %s\r\n"
		"Sec-WebSocket-Version: 13\r\n\r\n"
	, websocket->url.path, websocket->url.hostname, websocket->url.port, websocket->key);

	_write(websocket->ssl, websocket->sockfd, request_message, strlen(request_message));
	websocket->connected = true;
	free(request_message);
}

void send_websocket_message(Websocket *websocket, const char *message) {
	unsigned char *data = NULL;
	size_t message_length = strlen(message);
	size_t data_length = message_length;
	unsigned char masking_key[5] = {0};

	for (int i = 0; i < 4; ++i) {
		masking_key[i] = ((rand() % 255) + 1);
	}

	if (message_length > 65535) {
		data_length += 14;
		data = allocate(NULL, -1, data_length, sizeof(char));
		data[1] = 255;

		for (int i = 0; i < 8; ++i) {
			data[2 + i] = (message_length >> ((7 - i) * 8)) & 0xFF;
		}
	} else if (message_length > 125) {
		data_length += 8;
		data = allocate(NULL, -1, data_length, sizeof(char));
		data[1] = 254;
		data[2] = (message_length >> 8) & 0xFF;
		data[3] = message_length & 0xFF;
	} else {
		data_length += 6;
		data = allocate(NULL, -1, data_length, sizeof(char));
		data[1] = 128 + message_length;
	}

	data[0] = WEBSOCKET_FRAME_MAGIC;
	strcpy(((char *) data) + data_length - message_length - 4, (char *) masking_key);

	for (int i = 0; i < message_length; ++i) {
		char ch = (message[i] ^ masking_key[i % 4]);
		strncpy(((char *) data) + data_length - message_length + i, &ch, 1);
	}

	++websocket->tbs_size;
	websocket->tbs = allocate(websocket->tbs, -1, websocket->tbs_size, sizeof(WebsocketTBS));
	websocket->tbs[websocket->tbs_size - 1].data = allocate(NULL, -1, data_length, sizeof(char));
	websocket->tbs[websocket->tbs_size - 1].size = data_length;
	memcpy(websocket->tbs[websocket->tbs_size - 1].data, data, data_length);

	free(data);
}


Websocket create_websocket(const char *url, const WebsocketMethods methods) {
	Websocket websocket = {0};
	websocket.methods = methods;

	initialize_websocket(&websocket, url);
	create_epoll(&websocket);

	return websocket;
}

static void initialize_websocket(Websocket *websocket, const char *url) {
	websocket->url = parse_url(url);
	websocket->sockfd = socket(AF_INET, SOCK_STREAM, 0);
	websocket->ssl = NULL;

	if (websocket->sockfd == -1) {
		throw_network("socket()", !!websocket->ssl);
	}

	int keepalive = 1;

	if (setsockopt(websocket->sockfd, SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof(int)) == -1) {
		throw_network("setsockopt()", !!websocket->ssl);
	}

	if (strcmp(websocket->url.protocol, "wss") == 0) {
		SSL_load_error_strings();
		SSL_library_init();

		websocket->ssl = SSL_new(SSL_CTX_new(SSLv23_client_method()));
		SSL_set_fd(websocket->ssl, websocket->sockfd);
	}
}

void connect_websocket(Websocket *websocket) {
	struct hostent *host = resolve_hostname(websocket->url.hostname);

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(websocket->url.port);
	memcpy(&addr.sin_addr, host->h_addr_list[0], (size_t) host->h_length);

	if (connect(websocket->sockfd, (const struct sockaddr *) &addr, sizeof(struct sockaddr_in)) == 0) {
		if (websocket->ssl) {
			SSL_connect(websocket->ssl);
		}

		struct epoll_event events[16];

		register_events(websocket->epollfd, websocket, EPOLLIN | EPOLLOUT);

		do {
			int num_events = epoll_wait(websocket->epollfd, events, 32, -1);

			if (num_events != 0) {
				handle_events(websocket, websocket->epollfd, events, num_events);
			}

			usleep(3000);
		} while (websocket->connected && !websocket->closed);
	} else {
		throw_network("connect()", !!websocket->ssl);
	}
}

void close_websocket(Websocket *websocket, const short code, const char *reason) {
	if (websocket->connected && !websocket->closed) {
		unregister_events(websocket->epollfd, websocket, EPOLLIN | EPOLLOUT);

		websocket->connected = false;
		websocket->closed = true;

		close(websocket->epollfd);
		close_socket(websocket->sockfd, websocket->ssl);
	}


	for (size_t i = 0; i < websocket->tbs_size; ++i) {
		free(websocket->tbs[i].data);
	}

	free(websocket->tbs);
	free_url(websocket->url);

	if (websocket->methods.onclose) {
		websocket->methods.onclose(code, reason);
	}
}
