#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <math.h>

#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <openssl/ssl.h>
#include <openssl/err.h>

#include <network.h>
#include <utils.h>

static struct {
	bool fin, rsv1, rsv2, rsv3, mask;
	char *payload;
	unsigned char opcode;
} __attribute__((packed)) websocket_data;

static struct hostent *resolve_hostname(char *hostname) {
	struct hostent *result = gethostbyname(hostname);

	if (result == NULL && strncmp(hostname, "www.", 4) == 0) {
		result = gethostbyname(hostname + 4);
	}

	return result;
}

static void close_socket(bool tls, int *sockfd, SSL **ssl, SSL_CTX **ssl_ctx) {
	if (tls && *ssl != NULL && *ssl_ctx != NULL) {
		SSL_shutdown(*ssl);
		SSL_CTX_free(*ssl_ctx);
		SSL_free(*ssl);
	}

	close(*sockfd);
}

static void throw(bool tls, char *value) {
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

static void parse_data(unsigned char *data) {
	size_t payload_length;
	unsigned char ends_at;

	websocket_data.fin = ((data[0] >> 7) & 0x1);
	websocket_data.rsv1 = ((data[0] >> 6) & 0x1);
	websocket_data.rsv2 = ((data[0] >> 5) & 0x1);
	websocket_data.rsv3 = ((data[0] >> 4) & 0x1);
	websocket_data.opcode = (unsigned char) (data[0] - (websocket_data.fin ? 128 : 0) - (websocket_data.rsv1 ? 64 : 0) - (websocket_data.rsv2 ? 32 : 0) - (websocket_data.rsv3 ? 16 : 0));

	websocket_data.mask = ((data[1] >> 7) & 0x1);
	payload_length = (websocket_data.mask ? (data[1] - 128) : data[1]);
	ends_at = 2;

	if (payload_length == 126) {
		payload_length = (data[2] << 8)  + data[3];
		ends_at = 3;
	} else if (payload_length == 127) {
		payload_length = (data[2] << 56) + (data[3] << 48) + (data[4] << 40) + (data[5] << 32) + (data[6] << 24) + (data[7] << 16) + (data[8] << 8) + data[9];
		ends_at = 9;
	}

	if (websocket_data.opcode == 0x1) {
		websocket_data.payload = allocate(websocket_data.payload, payload_length + 1, sizeof(char));
		strncpy(websocket_data.payload, ((char *) data) + ends_at, payload_length);
	}
}

void connect_websocket(Websocket *websocket) {
	int sockfd;
	struct sockaddr_in addr;
	struct hostent *host = NULL;
	SSL *ssl = NULL;
	SSL_CTX *ssl_ctx = NULL;

	Split splitter;
	char hostname[1024] = {0}, *path = NULL;
	bool tls;
	unsigned short port;

	tls = (strncmp(websocket->url, "wss", 3) == 0);
	port = (websocket->port ? websocket->port : (tls ? 443 : 80));

	splitter = split(websocket->url, "/");
	path = allocate(path, calculate_join(splitter.data + 3, splitter.size - 3, "/") + 2, sizeof(char));
	path[0] = '/';
	join(splitter.data + 3, path + 1, splitter.size - 3, "/");
	strcpy(hostname, splitter.data[2]);
	split_free(&splitter);

	host = resolve_hostname(hostname);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	memcpy(&addr.sin_addr, host->h_addr_list[0], (size_t) host->h_length);

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		throw(tls, "socket()");
	}

	if (connect(sockfd, (const struct sockaddr *) &addr, sizeof(struct sockaddr_in)) == 0) {
		Split line_splitter, header_splitter;

		char buffer[1024] = {0};
		char status_str[4] = {0};
		char accept_key[512] = {0};
		char *response_message = NULL;
		char *request_message = NULL;

		size_t request_message_length, nread, i;
		size_t response_message_length = 0;

		unsigned short status;

		if (tls) {
			SSL_load_error_strings();
			SSL_library_init();

			ssl_ctx = SSL_CTX_new(TLS_client_method());
			ssl = SSL_new(ssl_ctx);
			SSL_set_fd(ssl, sockfd);

			SSL_connect(ssl);
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

		if ((tls ? SSL_write(ssl, request_message, (int) request_message_length) : write(sockfd, request_message, request_message_length)) <= 0) {
			close_socket(tls, &sockfd, &ssl, &ssl_ctx);
			throw(tls, "write()");
		}

		while ((nread = (size_t) (tls ? SSL_read(ssl, buffer, 1023) : read(sockfd, buffer, 1023))) > 0) {
			if (errno != 0) {
				close_socket(tls, &sockfd, &ssl, &ssl_ctx);
				throw(tls, "read()");
			} else {
				response_message_length += nread;
				response_message = allocate(response_message, response_message_length + 1, sizeof(char));
				strncat(response_message, buffer, nread);
			}
		}

		line_splitter = split(response_message, "\r\n");
		strncpy(status_str, line_splitter.data[0] + 9, 3);
		status = (unsigned short) atoi(status_str);

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

			if (websocket->onstart != NULL) {
				parse_data(data);
				websocket->onmessage(payload);

				free(websocket_data.payload);
			}

			split_free(&line_splitter);
			free(request_message);
			free(response_message);
			free(data);

			close_socket(tls, &sockfd, &ssl, &ssl_ctx);
		} else {
			split_free(&line_splitter);
			free(request_message);

			close_socket(tls, &sockfd, &ssl, &ssl_ctx);
		}
	} else {
		close_socket(tls, &sockfd, &ssl, &ssl_ctx);
		throw(tls, "connect()");
	}
}
