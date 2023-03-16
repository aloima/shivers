#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <openssl/ssl.h>
#include <openssl/err.h>

#include <network.h>
#include <utils.h>

static struct hostent *resolve_hostname(char *hostname) {
	struct hostent *result = gethostbyname(hostname);

	if (result == NULL && strncmp(hostname, "www.", 4) == 0) {
		result = gethostbyname(hostname + 4);
	}

	return result;
}

static void close_socket(bool tls, int *sockfd, SSL **ssl, SSL_CTX **ssl_ctx) {
	if (tls) {
		SSL_shutdown(*ssl);
		SSL_CTX_free(*ssl_ctx);
		SSL_free(*ssl);
	}

	close(*sockfd);
}

static void give_error(bool tls, char *value) {
	unsigned long tls_error;

	if (errno != 0) {
		perror(value);
	} else if (tls && (tls_error = ERR_get_error()) != 0) {
		char message[1024];

		ERR_error_string(tls_error, message);
		fprintf(stderr, "%s: %s\n", value, message);
	}
}

void response_free(Response *response) {
	size_t i;

	for (i = 0; i < response->header_size; ++i) {
		free(response->headers[i].name);
		free(response->headers[i].value);
	}

	free(response->headers);
	free(response->data);
	free(response->status.message);
}

Response request(RequestConfig config) {
	Response response;
	int sockfd;
	struct sockaddr_in addr;
	struct hostent *host = NULL;
	SSL *ssl = NULL;
	SSL_CTX *ssl_ctx = NULL;

	Split splitter;
	char hostname[16384], path[49152], *_path = NULL;
	bool tls;
	short port;

	memset(&splitter, 0, sizeof(splitter));
	memset(&response, 0, sizeof(response));
	tls = (strncmp(config.url, "https", 5) == 0);
	port = (tls ? 443 : 80);

	splitter = split(config.url, "/");
	_path = join(splitter.data + 3, splitter.size - 3, "/");
	strcpy(hostname, splitter.data[2]);
	sprintf(path, "/%s", _path == NULL ? "" : _path);
	free(_path);
	split_free(&splitter);

	memset(&addr, 0, sizeof(addr));
	host = resolve_hostname(hostname);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	memcpy(&addr.sin_addr, host->h_addr_list[0], (size_t) host->h_length);

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		give_error(tls, "socket()");
		return response;
	}

	if (connect(sockfd, (const struct sockaddr *) &addr, sizeof(addr)) == 0) {
		Split line_splitter, status_splitter, header_splitter;
		char buffer[1024], *response_message = NULL, *request_message = NULL, tmp[262144];
		size_t request_message_length, response_message_length = 0, nread, i, response_data_length = 0;
		memset(&buffer, 0, 1024);
		memset(&line_splitter, 0, sizeof(line_splitter));
		memset(&status_splitter, 0, sizeof(status_splitter));
		memset(&header_splitter, 0, sizeof(header_splitter));

		if (tls) {
			SSL_load_error_strings();
			SSL_library_init();

			ssl_ctx = SSL_CTX_new(TLS_client_method());
			ssl = SSL_new(ssl_ctx);
			SSL_set_fd(ssl, sockfd);

			SSL_connect(ssl);
		}

		request_message_length = (size_t) sprintf(tmp,
			"GET %s HTTP/1.1\r\n"
			"Host: %s:%d\r\n"
			"Accept: */*\r\n"
			"Connection: close\r\n\r\n"
		, path, hostname, port);

		request_message = allocate(request_message, request_message_length + 1, sizeof(char));
		strncpy(request_message, tmp, request_message_length);

		if ((tls ? SSL_write(ssl, request_message, (int) request_message_length) : write(sockfd, request_message, request_message_length)) <= 0) {
			give_error(tls, "write()");
			close_socket(tls, &sockfd, &ssl, &ssl_ctx);
			return response;
		}

		while ((nread = (size_t) (tls ? SSL_read(ssl, buffer, 1023) : read(sockfd, buffer, 1023))) > 0) {
			if (errno != 0) {
				give_error(tls, "read()");
				close_socket(tls, &sockfd, &ssl, &ssl_ctx);
				return response;
			} else {
				response_message_length += nread;
				response_message = allocate(response_message, response_message_length + 1, sizeof(char));
				strncat(response_message, buffer, nread);
			}
		}

		line_splitter = split(response_message, "\r\n");
		status_splitter = split(line_splitter.data[0], " ");
		response.status.code = (short) atoi(status_splitter.data[1]);
		response.status.message = join(status_splitter.data + 2, status_splitter.size - 2, " ");
		split_free(&status_splitter);

		for (i = 1; i < line_splitter.size; ++i) {
			if (line_splitter.data[i][0] == 0) {
				break;
			} else {
				size_t name_length, value_length;

				header_splitter = split(line_splitter.data[i], ": ");
				name_length = strlen(header_splitter.data[0]);
				value_length = strlen(header_splitter.data[1]);

				response.headers = allocate(response.headers, i, sizeof(Header));
				response.headers[i - 1].name = allocate(NULL, name_length + 1, sizeof(char));
				response.headers[i - 1].value = allocate(NULL, value_length + 1, sizeof(char));
				strncpy(response.headers[i - 1].name, header_splitter.data[0], name_length);
				strncpy(response.headers[i - 1].value, header_splitter.data[1], value_length);
				++response.header_size;

				split_free(&header_splitter);
			}
		}

		for (; i < line_splitter.size; ++i) {
			if (line_splitter.data[i][0] != 0) {
				bool last_line = ((i + 1) == line_splitter.size);
				size_t line_length = strlen(line_splitter.data[i]);
				response_data_length += line_length + (!last_line ? 2 : 0);
				response.data = allocate(response.data, response_data_length + 1, sizeof(char));
				strncat(response.data, line_splitter.data[i], line_length);

				if (!last_line) {
					strncat(response.data, "\r\n", 2);
				}
			}
		}

		split_free(&line_splitter);
		free(response_message);
		free(request_message);

		close_socket(tls, &sockfd, &ssl, &ssl_ctx);
	} else {
		give_error(tls, "connect()");
	}

	return response;
}
