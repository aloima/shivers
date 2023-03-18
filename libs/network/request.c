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

static char check_config(RequestConfig config) {
	char *allowed_methods[] = { "GET", "POST", "PUT", "DELETE", "PATCH" };
	size_t i, allowed_methods_length = (sizeof(allowed_methods) / sizeof(char *));
	bool is_allowed_method = false;

	if (config.method == NULL || config.url == NULL) {
		fprintf(stderr, "request(): url and method members are necessary\n");
		exit(EXIT_FAILURE);
	}

	for (i = 0; i < allowed_methods_length; ++i) {
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
	Response response = { false, { 0, NULL }, NULL, NULL, 0 };
	int sockfd;
	struct sockaddr_in addr;
	struct hostent *host = NULL;
	SSL *ssl = NULL;
	SSL_CTX *ssl_ctx = NULL;

	Split splitter;
	char hostname[1024] = {0}, *path = NULL;
	bool tls;
	unsigned short port;

	check_config(config);

	tls = (strncmp(config.url, "https", 5) == 0);
	port = (config.port ? config.port : (tls ? 443 : 80));

	splitter = split(config.url, "/");
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
		Split line_splitter, status_splitter, header_splitter;
		char buffer[1024] = {0}, *response_message = NULL, *request_message = NULL;
		size_t request_message_length, status_message_length, nread, i;
		size_t response_message_length = 0;
		size_t response_data_length = 0;

		if (tls) {
			SSL_load_error_strings();
			SSL_library_init();

			ssl_ctx = SSL_CTX_new(TLS_client_method());
			ssl = SSL_new(ssl_ctx);
			SSL_set_fd(ssl, sockfd);

			SSL_connect(ssl);
		}

		request_message_length = 55 + strlen(config.method) + strlen(path) + strlen(hostname) + sizeof(port);
		request_message = allocate(request_message, request_message_length + 1, sizeof(char));

		sprintf(request_message,
			"%s %s HTTP/1.1\r\n"
			"Host: %s:%d\r\n"
			"Accept: */*\r\n"
			"Connection: close\r\n\r\n"
		, config.method, path, hostname, port);

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
		status_splitter = split(line_splitter.data[0], " ");
		status_message_length = calculate_join(status_splitter.data + 2, status_splitter.size - 2, " ");
		response.status.code = (short) atoi(status_splitter.data[1]);
		response.status.message = allocate(response.status.message, status_message_length + 1, sizeof(char));
		join(status_splitter.data + 2, response.status.message, status_splitter.size - 2, " ");
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
		free(request_message);

		close_socket(tls, &sockfd, &ssl, &ssl_ctx);
	} else {
		close_socket(tls, &sockfd, &ssl, &ssl_ctx);
		throw(tls, "connect()");
	}

	return response;
}
