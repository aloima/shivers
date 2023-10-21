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

static char check_config(struct RequestConfig config) {
	const char *allowed_methods[] = { "GET", "POST", "PUT", "DELETE", "PATCH" };
	const size_t allowed_methods_length = (sizeof(allowed_methods) / sizeof(char *));
	bool is_allowed_method = false;

	if (config.method == NULL || config.url == NULL) {
		throw("request(): url and method members are necessary");
	}

	for (unsigned char i = 0; i < allowed_methods_length; ++i) {
		if (strcmp(config.method, allowed_methods[i]) == 0) {
			is_allowed_method = true;
			break;
		}
	}

	if (!is_allowed_method) {
		throw("request(): %s is invalid method", config.method);
	}

	return 1;
}

void response_free(struct Response *response) {
	for (unsigned char i = 0; i < response->header_size; ++i) {
		free(response->headers[i].name);
		free(response->headers[i].value);
	}

	free(response->headers);
	free(response->data);
	free(response->status.message);
}

// TODO: allocate request_message variable isntead of static array

struct Response request(struct RequestConfig config) {
	struct Response response = { false, { 0, NULL }, NULL, NULL, 0 };
	int sockfd;
	struct sockaddr_in addr;
	struct hostent *host = NULL;
	SSL *ssl = NULL;
	SSL_CTX *ssl_ctx = NULL;

	check_config(config);

	char *encoded_url = percent_encode(config.url);
	struct URL url = parse_url(encoded_url);
	bool tls = (url.port == 443);

	free(encoded_url);

	host = resolve_hostname(url.hostname);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(url.port);
	memcpy(&addr.sin_addr, host->h_addr_list[0], (size_t) host->h_length);

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		throw_network("socket()", tls);
	}

	if (connect(sockfd, (const struct sockaddr *) &addr, sizeof(struct sockaddr_in)) == 0) {
		char request_message[12288] = {0};
		size_t request_message_length;
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

		if (config.body.is_formdata) {
			const char *boundary = config.body.payload.formdata.boundary;
			const size_t boundary_length = strlen(boundary);

			char *data = NULL;
			size_t data_length = 0;

			char separator[32];
			sprintf(separator, "--%s\r\n", boundary);

			const unsigned short field_size = config.body.payload.formdata.field_size;

			for (unsigned short i = 0; i < field_size; ++i) {
				const struct FormDataField field = config.body.payload.formdata.fields[i];

				char line[128];
				size_t line_length = 0;
				const size_t disposition_length = ((field.filename ? (54 + strlen(field.filename)) : 41) + strlen(field.name));
				size_t field_length = (4 + boundary_length + disposition_length);

				data = allocate(data, -1, data_length + field_length + 1, sizeof(char));
				memcpy(data + data_length, separator, 4 + boundary_length);

				if (field.filename) {
					sprintf(line, "Content-Disposition: form-data; name=\"%s\"; filename=\"%s\"\r\n", field.name, field.filename);
				} else {
					sprintf(line, "Content-Disposition: form-data; name=\"%s\"\r\n", field.name);
				}

				memcpy(data + data_length + 4 + boundary_length, line, disposition_length);

				const unsigned char field_header_size = field.header_size;

				for (unsigned char a = 0; a < field_header_size; ++a) {
					const struct Header header = field.headers[a];
					sprintf(line, "%s: %s\r\n", header.name, header.value);
					line_length = strlen(line);

					data = allocate(data, -1, data_length + field_length + line_length + 1, sizeof(char));
					memcpy(data + data_length + field_length, line, line_length);
					field_length += line_length;
				}

				data = allocate(data, -1, data_length + field_length + 2 + 1, sizeof(char));
				memcpy(data + data_length + field_length, "\r\n", 2);
				field_length += 2;

				data = allocate(data, -1, data_length + field_length + field.data_size + 2 + 1, sizeof(char));
				memcpy(data + data_length + field_length, field.data, field.data_size);
				field_length += field.data_size;

				memcpy(data + data_length + field_length, "\r\n", 2);
				field_length += 2;

				data_length += field_length;
			}

			data = allocate(data, -1, 4 + boundary_length + data_length + 1, sizeof(char));
			sprintf(separator, "--%s--", boundary);
			memcpy(data + data_length, separator, (4 + boundary_length));
			data_length += (4 + boundary_length);

			sprintf(request_message, (
				"%s %s HTTP/1.1\r\n"
				"Host: %s:%d\r\n"
				"Accept: */*\r\n"
				"Connection: close\r\n"
				"Content-Type: multipart/form-data; boundary=%s\r\n"
				"Content-Length: %ld\r\n"
				"%s\r\n"
			), config.method, url.path, url.hostname, url.port, boundary, data_length, header_text);

			request_message_length = strlen(request_message);
			memcpy(request_message + request_message_length, data, data_length);
			request_message_length += data_length;

			free(data);
		} else if (config.body.payload.data != NULL) {
			char *body = config.body.payload.data;

			sprintf(request_message, (
				"%s %s HTTP/1.1\r\n"
				"Host: %s:%d\r\n"
				"Accept: */*\r\n"
				"Connection: close\r\n"
				"Content-Length: %ld\n"
				"%s\r\n"
				"%s"
			), config.method, url.path, url.hostname, url.port, strlen(body), header_text, body);

			request_message_length = strlen(request_message);
		} else {
			sprintf(request_message,
				"%s %s HTTP/1.1\r\n"
				"Host: %s:%d\r\n"
				"Accept: */*\r\n"
				"Connection: close\r\n"
				"%s\r\n"
			, config.method, url.path, url.hostname, url.port, header_text);

			request_message_length = strlen(request_message);
		}

		free_url(url);

		if (_write(ssl, sockfd, request_message, request_message_length) <= 0) {
			close_socket(sockfd, ssl);
			throw_network("write()", tls);
		}

		size_t read_size = 0;
		size_t response_message_length = 0;

		while ((read_size = _read(ssl, sockfd, buffer, 1023)) > 0) {
			if (errno != 0) {
				close_socket(sockfd, ssl);
				throw_network("read()", tls);
			} else {
				response_message_length += read_size;
				response_message = allocate(response_message, response_message_length - read_size + 1, response_message_length + 1, sizeof(char));
				strncat(response_message, buffer, read_size);
			}
		}

		Split line_splitter = split(response_message, "\r\n");
		Split status_splitter = split(line_splitter.data[0], " ");
		const size_t status_message_length = calculate_join(status_splitter.data + 2, status_splitter.size - 2, " ");

		response.status.code = atoi(status_splitter.data[1]);
		response.status.message = allocate(NULL, -1, status_message_length + 1, sizeof(char));
		join(status_splitter.data + 2, response.status.message, status_splitter.size - 2, " ");

		split_free(&status_splitter);

		size_t i = 0;

		for (i = 1; i < line_splitter.size; ++i) {
			if (line_splitter.data[i][0] == 0) {
				break;
			} else {
				Split header_splitter = split(line_splitter.data[i], ": ");

				response.headers = allocate(response.headers, -1, i, sizeof(struct Header));
				struct Header *header = &response.headers[i - 1];

				const char *name = header_splitter.data[0];
				const char *value = header_splitter.data[1];
				const size_t name_length = strlen(name);
				const size_t value_length = strlen(value);

				header->name = allocate(NULL, -1, name_length + 1, sizeof(char));
				header->value = allocate(NULL, -1, value_length + 1, sizeof(char));
				strncpy(header->name, name, name_length);
				strncpy(header->value, value, value_length);
				++response.header_size;

				split_free(&header_splitter);
			}
		}

		size_t response_data_length = 0;
		const char *transfer_encoding_value = get_header(response.headers, response.header_size, "transfer-encoding").value;

		if (transfer_encoding_value != NULL && strcmp(transfer_encoding_value, "chunked") == 0) {
			size_t response_read_data_length = 0;
			response_data_length = ahtoi(line_splitter.data[i + 1]);
			response.data = allocate(NULL, 0, response_data_length + 1, sizeof(char));
			i += 2;

			while (i < line_splitter.size && response_read_data_length != response_data_length) {
				const size_t line_length = strlen(line_splitter.data[i]);
				strncat(response.data, line_splitter.data[i], line_length);
				response_read_data_length += line_length;

				++i;
			}

			response.data[response_data_length - 1] = '\0';
		} else {
			while (i < line_splitter.size) {
				if (line_splitter.data[i][0] != 0) {
					const bool last_line = ((i + 1) == line_splitter.size);
					const size_t line_length = strlen(line_splitter.data[i]);
					response_data_length += (line_length + !last_line);
					response.data = allocate(response.data, response_data_length - line_length - !last_line + 1, response_data_length + 1, sizeof(char));
					strncat(response.data, line_splitter.data[i], line_length);

					if (!last_line) {
						strncat(response.data, "\n", 1);
					}
				}

				++i;
			}
		}

		split_free(&line_splitter);
		free(response_message);

		close_socket(sockfd, ssl);
	} else {
		close_socket(sockfd, ssl);
		throw_network("connect()", tls);
	}

	return response;
}
