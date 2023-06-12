#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>

#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <openssl/ssl.h>
#include <openssl/err.h>

#include <network.h>
#include <utils.h>

static bool fin, rsv1, rsv2, rsv3, mask;
static unsigned char opcode;

static struct hostent *resolve_hostname(char *hostname) {
	struct hostent *result = gethostbyname(hostname);

	if (result == NULL && strncmp(hostname, "www.", 4) == 0) {
		result = gethostbyname(hostname + 4);
	}

	return result;
}

static void close_socket(Websocket *websocket) {
	if (websocket->ssl != NULL) {
		SSL_shutdown(websocket->ssl);
		SSL_CTX_free(SSL_get_SSL_CTX(websocket->ssl));
		SSL_free(websocket->ssl);
	}

	close(websocket->sockfd);

	if (websocket->onclose) {
		websocket->onclose();
	}
}

static void throw(char *value, bool tls) {
	unsigned long tls_error;

	if (errno != 0) {
		perror(value);
		exit(EXIT_FAILURE);
	} else if (tls && (tls_error = ERR_get_error()) != 0) {
		char message[1024];

		ERR_error_string(tls_error, message);
		fprintf(stderr, "%s: %s\n", value, message);
		exit(EXIT_FAILURE);
	}
}

static char *parse_data(unsigned char *data, Websocket *websocket) {
	size_t payload_length;
	unsigned char ends_at;

	fin = ((data[0] >> 7) & 0x1);
	rsv1 = ((data[0] >> 6) & 0x1);
	rsv2 = ((data[0] >> 5) & 0x1);
	rsv3 = ((data[0] >> 4) & 0x1);
	opcode = (data[0] & 0xF);

	mask = ((data[1] >> 7) & 0x1);
	payload_length = (data[1] & 0x7F);
	ends_at = 2;

	if (payload_length == 126) {
		payload_length = (size_t) ((data[2] << 8) | data[3]);
		ends_at = 4;
	} else if (payload_length == 127) {
		payload_length = (((unsigned long) data[2] << 56) | ((unsigned long) data[3] << 48) | ((unsigned long) data[4] << 40) | ((unsigned long) data[5] << 32) | ((unsigned long) data[6] << 24) | ((unsigned long) data[7] << 16) | ((unsigned long) data[8] << 8) | (unsigned long) data[9]);
		ends_at = 10;
	}

	if (mask == 0x1) {
		ends_at += 4;
	}

	char *payload = calloc(payload_length + 1, sizeof(char));

	switch (opcode) {
		case 0x1:
			strncpy(payload, ((char *) data) + ends_at, payload_length);
			return payload;

		case 0x8:
			close_socket(websocket);
			break;
	}

	return NULL;
}

void send_websocket_message(Websocket *websocket, const char *message) {
	int res;

	if (websocket->ssl != NULL) {
		res = SSL_write(websocket->ssl, message, strlen(message));
	} else {
		res = write(websocket->sockfd, message, strlen(message));
	}

	if (res < 0) {
		throw("send_websocket_message()", !!(websocket->ssl));
	}
}

void connect_websocket(Websocket *websocket) {
	char hostname[1024] = {0};

	Split splitter = split(websocket->url, "/");
	bool tls = (strncmp(websocket->url, "wss", 3) == 0);
	unsigned short port = (websocket->port ? websocket->port : (tls ? 443 : 80));

	char *path = allocate(NULL, calculate_join(splitter.data + 3, splitter.size - 3, "/") + 2, sizeof(char));
	path[0] = '/';

	join(splitter.data + 3, path + 1, splitter.size - 3, "/");
	strcpy(hostname, splitter.data[2]);
	split_free(&splitter);

	struct hostent *host = resolve_hostname(hostname);

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);

	memcpy(&addr.sin_addr, host->h_addr_list[0], (size_t) host->h_length);

	websocket->sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (websocket->sockfd == -1) {
		throw("socket()", tls);
	}

	if (connect(websocket->sockfd, (const struct sockaddr *) &addr, sizeof(struct sockaddr_in)) == 0) {
		char buffer[1024] = {0};
		char status_str[4] = {0};
		char accept_key[512] = {0};
		char *response_message = NULL;
		char *request_message = NULL;

		size_t request_message_length, nread, i;
		size_t response_message_length = 0;

		websocket->ssl = NULL;

		if (tls) {
			SSL_load_error_strings();
			SSL_library_init();

			websocket->ssl = SSL_new(SSL_CTX_new(TLS_client_method()));
			SSL_set_fd(websocket->ssl, websocket->sockfd);

			SSL_connect(websocket->ssl);
		}

		request_message_length = 200 + strlen(path) + strlen(hostname) + sizeof(port);
		request_message = allocate(request_message, request_message_length + 1, sizeof(char));

		sprintf(request_message,
			"GET %s HTTP/1.1\r\n"
			"Host: %s:%d\r\n"
			"Accept: */*\r\n"
			"Connection: Upgrade\r\n"
			"Upgrade: websocket\r\n"
			"Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
			"Sec-WebSocket-Version: 13\r\n\r\n"
		, path, hostname, port);

		free(path);

		if ((tls ? SSL_write(websocket->ssl, request_message, (int) request_message_length) : write(websocket->sockfd, request_message, request_message_length)) <= 0) {
			close_socket(websocket);
			throw("write()", tls);
		}

		while ((nread = (size_t) (tls ? SSL_read(websocket->ssl, buffer, 1023) : read(websocket->sockfd, buffer, 1023))) > 0) {
			if (errno != 0) {
				close_socket(websocket);
				throw("read()", tls);
			} else {
				response_message_length += nread;
				response_message = allocate(response_message, response_message_length + 1, sizeof(char));
				strncat(response_message, buffer, nread);
			}
		}

		Split header_splitter;
		Split line_splitter = split(response_message, "\r\n");
		strncpy(status_str, line_splitter.data[0] + 9, 3);

		unsigned short status = atoi(status_str);

		if (status == 101) {
			unsigned char *data = NULL;
			size_t data_length = 0;

			for (i = 1; i < line_splitter.size; ++i) {
				if (line_splitter.data[i][0] == 0) {
					break;
				} else {
					size_t name_length, value_length;
					char header_name[256] = {0};
					memset(&header_name, 0, 256);

					header_splitter = split(line_splitter.data[i], ": ");
					name_length = strlen(header_splitter.data[0]);
					strtolower(header_name, header_splitter.data[0]);

					if (strncmp(header_name, "sec-websocket-accept", name_length) == 0) {
						value_length = strlen(header_splitter.data[1]);
						strncpy(accept_key, header_splitter.data[1], value_length);
					}

					split_free(&header_splitter);
				}
			}

			if (websocket->onstart != NULL) {
				websocket->onstart();
			}

			while (i < line_splitter.size) {
				if (line_splitter.data[i][0] != 0) {
					bool last_line = ((i + 1) == line_splitter.size);
					size_t line_length = strlen(line_splitter.data[i]);
					data_length += (line_length + (!last_line ? 2 : 0));
					data = allocate(data, data_length + 1, sizeof(char));
					strncat((char *) data, line_splitter.data[i], line_length);

					if (!last_line) {
						strncat((char *) data, "\r\n", 2);
					}
				}

				++i;
			}

			if (websocket->onmessage != NULL) {
				char *message = parse_data(data, websocket);
				websocket->onmessage(message);
				free(message);
			}

			split_free(&line_splitter);
			free(request_message);
			free(response_message);
			free(data);

			close_socket(websocket);
		} else {
			split_free(&line_splitter);
			free(request_message);

			close_socket(websocket);
		}
	} else {
		close_socket(websocket);
		throw("connect()", tls);
	}
}
