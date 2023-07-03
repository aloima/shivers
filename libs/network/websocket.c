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
static void handle_events(Websocket *websocket, int epoll_fd, struct epoll_event *events, size_t num_events);

static void switch_protocols(Websocket *websocket);
static void check_response(const char *response, const char *key);
static WebsocketFrame parse_data(unsigned char *data, Websocket *websocket);

static void initialize_websocket(Websocket *websocket, const char *url);
static void close_websocket(Websocket *websocket, short close_code);



static void create_epoll(Websocket *websocket) {
	websocket->epollfd = epoll_create1(0);

	if (websocket->epollfd == -1) {
		throw("epoll_create1()", false);
	}
}

static void register_events(int epoll_fd, Websocket *websocket, uint32_t event_flags) {
	struct epoll_event event;
	event.data.fd = websocket->sockfd;
	event.events = event_flags;

	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, websocket->sockfd, &event) == -1) {
		throw("epoll_ctl()", !!websocket->ssl);
	}
}

static void handle_events(Websocket *websocket, int epoll_fd, struct epoll_event *events, size_t num_events) {
	char *message = NULL;
	char buffer[4096] = {0};
	size_t message_length = 0;

	for (int i = 0; i < num_events; ++i) {
		uint32_t event = events[i].events;

		if (events[i].data.fd == websocket->sockfd) {
			if (event & EPOLLERR || event & EPOLLHUP) {
				throw("socket error or hang up", false);
			} else if (event & EPOLLIN) {
				size_t received = (websocket->ssl ? SSL_read(websocket->ssl, buffer, 4095) : read(websocket->sockfd, buffer, 4095));

				if (received > 0) {
					message_length += received;
					message = allocate(message, message_length + 1, sizeof(char));
					strncpy(message, buffer, received);

					if ((i + 1) == num_events) {
						if (strncmp(message, "HTTP", 4) == 0) {
							if (strncmp(message + 9, "101", 3) == 0) {
								check_response(message, websocket->key);
								free(websocket->key);

								if (websocket->methods.onstart) {
									websocket->methods.onstart();
								}
							} else {
								throw("invalid http status code", !!websocket->ssl);
							}
						} else {
							WebsocketFrame frame = parse_data((unsigned char *) message, websocket);

							if (websocket->methods.onmessage) {
								websocket->methods.onmessage(frame);
							}

							free(frame.payload);
						}
					}

					message_length = 0;
					free(message);
					message = NULL;
				} else {
					throw("failed to receive data", true);
				}
			} else if (event & EPOLLOUT) {
				if (!websocket->connected) {
					switch_protocols(websocket);
				} else if (websocket->tbs_size != 0) {
					WebsocketTBS tbs = websocket->tbs[0];
					bool err;

					if (websocket->ssl != NULL) {
						err = (SSL_write(websocket->ssl, tbs.data, tbs.size) <= 0);
					} else {
						err = (write(websocket->sockfd, tbs.data, tbs.size) < 0);
					}

					for (int i = 0; i < websocket->tbs_size; ++i) {
						if ((i + 1) != websocket->tbs_size) {
							websocket->tbs[i].data = allocate(websocket->tbs[i].data, websocket->tbs[i + 1].size + 1, sizeof(char));
							websocket->tbs[i].size = websocket->tbs[i + 1].size;
							memcpy(websocket->tbs[i].data, websocket->tbs[i + 1].data, websocket->tbs[i + 1].size);
						}
					}

					free(websocket->tbs[websocket->tbs_size - 1].data);
					websocket->tbs = allocate(websocket->tbs, websocket->tbs_size - 1, sizeof(WebsocketTBS));
					--websocket->tbs_size;

					if (err) {
						throw("send_websocket_message()", !!websocket->ssl);
					}
				}
			}
		}
	}
}

static void check_response(const char *response, const char *key) {
	Split splitter = split((char *) response, "\r\n");

	for (int i = 0; i < splitter.size; ++i) {
		if (strstr(splitter.data[i], ":") != NULL) {
			Split line_splitter = split(splitter.data[i], ":");
			char header_name[128] = {0};
			strtolower(header_name, line_splitter.data[0]);

			if (strcmp(header_name, "sec-websocket-accept") == 0) {
				char *value = ltrim(line_splitter.data[1]);

				const char* websocket_guid = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
				size_t final_length = strlen(key) + strlen(websocket_guid);
				char *final = allocate(NULL, final_length + 1, sizeof(char));
				strcat(final, key);
				strcat(final, websocket_guid);

				unsigned char *sha1_hash = allocate(NULL, SHA_DIGEST_LENGTH + 1, sizeof(char));
				SHA1((unsigned char *) final, final_length, sha1_hash);
				char *result = base64_encode((char *) sha1_hash);

				if (strcmp(result, value) != 0) {
					throw("switch_protocols(): unmatched websocket keys", false);
				}

				free(sha1_hash);
				free(final);
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

	websocket->key = base64_encode((char *) key_data);

	char *request_message = allocate(NULL, 512, sizeof(char));

	sprintf(request_message,
		"GET %s HTTP/1.1\r\n"
		"Host: %s:%d\r\n"
		"Upgrade: websocket\r\n"
		"Connection: Upgrade\r\n"
		"Sec-WebSocket-Key: %s\r\n"
		"Sec-WebSocket-Version: 13\r\n\r\n"
	, websocket->url.path, websocket->url.hostname, websocket->url.port, websocket->key);

	if ((websocket->ssl ? SSL_write(websocket->ssl, request_message, strlen(request_message)) : write(websocket->sockfd, request_message, strlen(request_message))) <= 0) {
		close_websocket(websocket, 0);
		throw("write()", !!websocket->ssl);
	}

	websocket->connected = true;
	free(request_message);
}

static WebsocketFrame parse_data(unsigned char *data, Websocket *websocket) {
	WebsocketFrame frame;
	unsigned char ends_at;

	frame.fin = ((data[0] >> 7) & 0x1);
	frame.rsv[0] = ((data[0] >> 6) & 0x1);
	frame.rsv[1] = ((data[0] >> 5) & 0x1);
	frame.rsv[2] = ((data[0] >> 4) & 0x1);
	frame.opcode = (data[0] & 0xF);

	frame.mask = ((data[1] >> 7) & 0x1);
	frame.payload_length = (data[1] & 0x7F);
	ends_at = 2;

	if (frame.payload_length == 126) {
		frame.payload_length = (size_t) ((data[2] << 8) | data[3]);
		ends_at = 4;
	} else if (frame.payload_length == 127) {
		frame.payload_length = (((unsigned long) data[2] << 56) | ((unsigned long) data[3] << 48) | ((unsigned long) data[4] << 40) | ((unsigned long) data[5] << 32) | ((unsigned long) data[6] << 24) | ((unsigned long) data[7] << 16) | ((unsigned long) data[8] << 8) | (unsigned long) data[9]);
		ends_at = 10;
	}

	if (frame.mask == 0x1) {
		ends_at += 4;
	}

	frame.payload = allocate(NULL, frame.payload_length + 1, sizeof(char));

	switch (frame.opcode) {
		case 0x1:
			strcpy(frame.payload, ((char *) data) + ends_at);
			break;

		case 0x8: {
			short close_code = 0;
			close_code |= (data[ends_at] << 8);
			close_code |= data[ends_at + 1];
			close_websocket(websocket, close_code);
			break;
		}
	}

	return frame;
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
		data = allocate(data, data_length, sizeof(char));
		data[1] = 255;

		for (int i = 0; i < 8; ++i) {
			data[2 + i] = (message_length >> ((7 - i) * 8)) & 0xFF;
		}
	} else if (message_length > 125) {
		data_length += 8;
		data = allocate(data, data_length, sizeof(char));
		data[1] = 254;
		data[2] = (message_length >> 8) & 0xFF;
		data[3] = message_length & 0xFF;
	} else {
		data_length += 6;
		data = allocate(data, data_length, sizeof(char));
		data[1] = message_length;
	}

	data[0] = WEBSOCKET_FRAME_MAGIC;
	strcpy(((char *) data) + data_length - message_length - 4, (char *) masking_key);

	for (int i = 0; i < message_length; ++i) {
		char ch = (message[i] ^ masking_key[i % 4]);
		strncpy(((char *) data) + data_length - message_length + i, &ch, 1);
	}

	++websocket->tbs_size;
	websocket->tbs = allocate(websocket->tbs, websocket->tbs_size, sizeof(WebsocketTBS));
	websocket->tbs[websocket->tbs_size - 1].data = allocate(NULL, data_length, sizeof(char));
	websocket->tbs[websocket->tbs_size - 1].size = data_length;
	memcpy(websocket->tbs[websocket->tbs_size - 1].data, data, data_length);

	free(data);
}



Websocket create_websocket(const char *url, const WebsocketMethods methods) {
	Websocket websocket;
	memset(&websocket, 0, sizeof(Websocket));

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
		throw("socket()", !!websocket->ssl);
	}

	int keepalive = 1;

	if (setsockopt(websocket->sockfd, SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof(int)) == -1) {
		throw("setsockopt()", !!websocket->ssl);
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

		register_events(websocket->epollfd, websocket, EPOLLIN | EPOLLOUT | EPOLLERR | EPOLLHUP);

		do {
			int num_events = epoll_wait(websocket->epollfd, events, 16, -1);

			if (num_events != 0) {
				handle_events(websocket, websocket->epollfd, events, num_events);
			}
		} while (websocket->connected);
	} else {
		throw("connect()", !!websocket->ssl);
	}
}

static void close_websocket(Websocket *websocket, short close_code) {
	close_socket(websocket->sockfd, websocket->ssl);
	close(websocket->epollfd);
	free_url(websocket->url);

	websocket->connected = false;

	if (websocket->methods.onclose) {
		websocket->methods.onclose((const short) close_code);
	}
}
