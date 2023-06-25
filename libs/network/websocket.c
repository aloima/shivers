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
#include <openssl/err.h>

#include <network.h>
#include <utils.h>

static void create_epoll(Websocket *websocket);
static void register_events(int epoll_fd, Websocket *websocket, uint32_t event_flags);
static void handle_events(Websocket *websocket, int epoll_fd, struct epoll_event *events, size_t num_events);

static void switch_protocols(Websocket *websocket);
static WebsocketFrame parse_data(unsigned char *data, Websocket *websocket);

static void initialize_websocket(Websocket *websocket, const char *url);
static void close_websocket(Websocket *websocket);



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
							if (websocket->methods.onstart) {
								websocket->methods.onstart();
							}
						} else {
							throw("invalid http status code", !!websocket->ssl);
						}
					} else if (websocket->methods.onmessage) {
						WebsocketFrame frame = parse_data((unsigned char *) message, websocket);
						websocket->methods.onmessage(frame);
						free(frame.payload);
					}

					message_length = 0;
					free(message);
					message = NULL;
				}
			} else {
				throw("failed to receive data", false);
			}
		} else if (event & EPOLLOUT) {
			if (!websocket->connected) {
				switch_protocols(websocket);
			}
		}
	}
}

static void switch_protocols(Websocket *websocket) {
	char *request_message = allocate(NULL, 512, sizeof(char));

	sprintf(request_message,
		"GET %s HTTP/1.1\r\n"
		"Host: %s:%d\r\n"
		"Accept: */*\r\n"
		"Connection: Upgrade\r\n"
		"Upgrade: websocket\r\n"
		"Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
		"Sec-WebSocket-Version: 13\r\n\r\n"
	, websocket->url.path, websocket->url.hostname, websocket->url.port);

	if ((websocket->ssl ? SSL_write(websocket->ssl, request_message, 512) : write(websocket->sockfd, request_message, 512)) <= 0) {
		close_websocket(websocket);
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
			strncpy(frame.payload, ((char *) data) + ends_at, frame.payload_length);
			break;

		case 0x8:
			close_websocket(websocket);
			break;
	}

	return frame;
}

void send_websocket_message(Websocket *websocket, const char *message) {
	unsigned char *data = NULL;	
	size_t message_length = strlen(message), data_length = message_length;

	if (message_length > 65535) {
		data_length += 10;
		data = allocate(data, data_length, sizeof(char));
		data[0] = 0x81; // fin (1) + rsvs (000) + op (0001)
		data[1] = 127; // mask (0) | length specifier 127 (1111111)
		data[2] = message_length >> 56;
		data[3] = (message_length >> 48) & 0xFF;
		data[4] = (message_length >> 40) & 0xFF;
		data[5] = (message_length >> 32) & 0xFF;
		data[6] = (message_length >> 24) & 0xFF;
		data[7] = (message_length >> 16) & 0xFF;
		data[8] = (message_length >> 8) & 0xFF;
		data[9] = message_length & 0xFF;
		strncpy(((char *) data) + 10, message, message_length);
	} else if (message_length > 125) {
		data_length += 4;
		data = allocate(data, data_length, sizeof(char));
		data[0] = 0x81; // fin (1) | rsvs (000) | op (0001)
		data[1] = 126; // mask (0) | length specifier 126 (1111110)
		data[2] = message_length >> 8;
		data[3] = message_length & 0xFF;
		strncpy(((char *) data) + 4, message, message_length);
	} else {
		data_length += 2;
		data = allocate(data, data_length, sizeof(char));
		data[0] = 0x81; // fin (1) | rsvs (000) | op (0001)
		data[1] = message_length; // mask (0) | length
		strncpy(((char *) data) + 2, message, message_length);
	}

	bool err;

	if (websocket->ssl != NULL) {
		err = (SSL_write(websocket->ssl, data, data_length) <= 0);
	} else {
		err = (write(websocket->sockfd, data, data_length) < 0);
	}

	free(data);

	if (err) {
		throw("send_websocket_message()", !!websocket->ssl);
	}
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

	if (strcmp(websocket->url.protocol, "wss") == 0) {
		SSL_load_error_strings();
		SSL_library_init();

		websocket->ssl = SSL_new(SSL_CTX_new(TLS_client_method()));
		SSL_set_fd(websocket->ssl, websocket->sockfd);
	}

	if (websocket->sockfd == -1) {
		throw("socket()", !!websocket->ssl);
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
			int num_events = epoll_wait(websocket->epollfd, events, 16, -1);
			handle_events(websocket, websocket->epollfd, events, num_events);
		} while (websocket->connected);
	} else {
		throw("connect()", !!websocket->ssl);
	}
}

static void close_websocket(Websocket *websocket) {
	close_socket(websocket->sockfd, websocket->ssl);
	close(websocket->epollfd);
	free_url(websocket->url);

	websocket->connected = 0;

	if (websocket->methods.onclose) {
		websocket->methods.onclose();
	}
}
