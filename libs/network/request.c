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

static char check_config(RequestConfig config) {
	char *allowed_methods[] = { "GET", "POST", "PUT", "DELETE", "PATCH" };
	size_t allowed_methods_length = (sizeof(allowed_methods) / sizeof(char *));
	bool is_allowed_method = false;

	if (config.method == NULL || config.url == NULL) {
		fprintf(stderr, "request(): url and method members are necessary\n");
		exit(EXIT_FAILURE);
	}

	for (size_t i = 0; i < allowed_methods_length; ++i) {
		if (strcmp(config.method, allowed_methods[i]) == 0) {
			is_allowed_method = true;
			break;
		}
	}

	if (!is_allowed_method) {
		fprintf(stderr, "request(): %s is invalid method\n", config.method);
		exit(EXIT_FAILURE);
	}

	return 1;
}

void response_free(Response *response) {
	for (size_t i = 0; i < response->header_size; ++i) {
		free(response->headers[i].name);
		free(response->headers[i].value);
	}

	free(response->headers);
	free(response->data);
	free(response->status.message);
}

Response request(RequestConfig config) {
	Response response = { false, { 0, NULL }, NULL, NULL, 0 };
	int sockfd;
	struct sockaddr_in addr;
	struct hostent *host = NULL;
	SSL *ssl = NULL;
	SSL_CTX *ssl_ctx = NULL;

	check_config(config);
	URL url = parse_url(config.url);
	bool tls = (url.port == 443);

	host = resolve_hostname(url.hostname);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(url.port);
	memcpy(&addr.sin_addr, host->h_addr_list[0], (size_t) host->h_length);

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		throw("socket()", tls);
	}

	if (connect(sockfd, (const struct sockaddr *) &addr, sizeof(struct sockaddr_in)) == 0) {
		char request_message[512];
		memset(&request_message, 0, sizeof(request_message));

		char buffer[1024] = {0}, *response_message = NULL;

		if (tls) {
			SSL_load_error_strings();
			SSL_library_init();

			ssl_ctx = SSL_CTX_new(TLS_client_method());
			ssl = SSL_new(ssl_ctx);
			SSL_set_fd(ssl, sockfd);

			SSL_connect(ssl);
		}

		char header_text[65536] = {0};

		for (int i = 0; i < config.header_size; ++i) {
			strcat(header_text, config.headers[i].name);
			strcat(header_text, ": ");
			strcat(header_text, config.headers[i].value);
			strcat(header_text, "\r\n");
		}

		sprintf(request_message,
			"%s %s HTTP/1.1\r\n"
			"Host: %s:%d\r\n"
			"Accept: */*\r\n"
			"Connection: close\r\n"
			"%s\r\n"
		, config.method, url.path, url.hostname, url.port, header_text);

		free_url(url);

		size_t request_message_length = strlen(request_message);

		if ((tls ? SSL_write(ssl, request_message, request_message_length) : write(sockfd, request_message, request_message_length)) <= 0) {
			close_socket(sockfd, ssl);
			throw("write()", tls);
		}

		size_t read_size = 0;
		size_t response_message_length = 0;

		while ((read_size = (size_t) (tls ? SSL_read(ssl, buffer, 1023) : read(sockfd, buffer, 1023))) > 0) {
			if (errno != 0) {
				close_socket(sockfd, ssl);
				throw("read()", tls);
			} else {
				response_message_length += read_size;
				response_message = allocate(response_message, response_message_length + 1, sizeof(char));
				strncat(response_message, buffer, read_size);
			}
		}

		Split line_splitter = split(response_message, "\r\n");
		Split status_splitter = split(line_splitter.data[0], " ");
		size_t status_message_length = calculate_join(status_splitter.data + 2, status_splitter.size - 2, " ");
		response.status.code = (short) atoi(status_splitter.data[1]);
		response.status.message = allocate(response.status.message, status_message_length + 1, sizeof(char));
		join(status_splitter.data + 2, response.status.message, status_splitter.size - 2, " ");
		split_free(&status_splitter);

		size_t i = 0;
		size_t response_data_length = 0;

		for (i = 1; i < line_splitter.size; ++i) {
			if (line_splitter.data[i][0] == 0) {
				break;
			} else {
				Split header_splitter = split(line_splitter.data[i], ": ");
				size_t name_length = strlen(header_splitter.data[0]);
				size_t value_length = strlen(header_splitter.data[1]);

				response.headers = allocate(response.headers, i, sizeof(Header));
				response.headers[i - 1].name = allocate(NULL, name_length + 1, sizeof(char));
				response.headers[i - 1].value = allocate(NULL, value_length + 1, sizeof(char));
				strncpy(response.headers[i - 1].name, header_splitter.data[0], name_length);
				strncpy(response.headers[i - 1].value, header_splitter.data[1], value_length);
				++response.header_size;

				split_free(&header_splitter);
			}
		}

		while (i < line_splitter.size) {
			if (line_splitter.data[i][0] != 0) {
				bool last_line = ((i + 1) == line_splitter.size);
				size_t line_length = strlen(line_splitter.data[i]);
				response_data_length += (line_length + (!last_line ? 2 : 0));
				response.data = allocate(response.data, response_data_length + 1, sizeof(char));
				strncat(response.data, line_splitter.data[i], line_length);

				if (!last_line) {
					strncat(response.data, "\r\n", 2);
				}
			}

			++i;
		}

		split_free(&line_splitter);
		free(response_message);

		close_socket(sockfd, ssl);
	} else {
		close_socket(sockfd, ssl);
		throw("connect()", tls);
	}

	return response;
}
