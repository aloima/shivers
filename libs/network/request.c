#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>

#if defined(_WIN32)
	#include <winsock2.h>
#elif defined(__linux__)
	#include <sys/socket.h>
	#include <unistd.h>
	#include <arpa/inet.h>
	#include <netdb.h>
#endif

#include <openssl/ssl.h>
#include <openssl/err.h>

#include <network.h>
#include <utils.h>

static void check_config(struct RequestConfig config) {
	const char *allowed_methods[] = { "GET", "POST", "PUT", "DELETE", "PATCH" };
	const unsigned long allowed_methods_length = (sizeof(allowed_methods) / sizeof(char *));
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

struct Response request(struct RequestConfig config) {
	struct Response response = {0};

	#if defined(_WIN32)
		SOCKET sockfd;
	#elif defined(__linux__)
		int sockfd;
	#endif

	struct sockaddr_in addr;
	struct hostent *host = NULL;
	SSL *ssl = NULL;
	SSL_CTX *ssl_ctx = NULL;

	check_config(config);

	char *encoded_url = percent_encode(config.url);
	struct URL url = parse_url(encoded_url);
	const bool tls = (url.port == 443);

	free(encoded_url);

	host = resolve_hostname(url.hostname);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(url.port);
	memcpy(&addr.sin_addr, host->h_addr_list[0], (unsigned long) host->h_length);

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		throw_network("socket()", tls);
	}

	if (connect(sockfd, (const struct sockaddr *) &addr, sizeof(struct sockaddr_in)) == 0) {
		char request_message_information[12288], *request_message = NULL;
		unsigned long request_message_length;
		char buffer[HTTP_BUFFER_SIZE + 1], *response_message = NULL;
		buffer[HTTP_BUFFER_SIZE] = 0;

		if (tls) {
			SSL_load_error_strings();
			SSL_library_init();

			ssl_ctx = SSL_CTX_new(TLS_client_method());

			// These settings are specified for shivers, if you're using this library independently, change them for what your needs
			SSL_CTX_set_cipher_list(ssl_ctx, "TLS_RSA_WITH_AES_256_CBC_SHA256");
			SSL_CTX_set_session_cache_mode(ssl_ctx, SSL_SESS_CACHE_OFF);
			SSL_CTX_set_options(ssl_ctx, SSL_OP_NO_TLSv1);

			ssl = SSL_new(ssl_ctx);
			SSL_set_fd(ssl, sockfd);

			SSL_connect(ssl);
		}

		char header_text[9216];
		header_text[0] = '\0';

		for (int i = 0; i < config.header_size; ++i) {
			strcat(header_text, config.headers[i].name);
			strcat(header_text, ": ");
			strcat(header_text, config.headers[i].value);
			strcat(header_text, "\r\n");
		}

		if (config.body.is_formdata) {
			const char *boundary = config.body.payload.formdata.boundary;
			const unsigned long boundary_length = strlen(boundary);

			char *data = NULL;
			unsigned long data_length = 0;

			char separator[32];
			sprintf(separator, "--%s\r\n", boundary);

			const unsigned short field_size = config.body.payload.formdata.field_size;

			for (unsigned short i = 0; i < field_size; ++i) {
				const struct FormDataField field = config.body.payload.formdata.fields[i];

				char line[128];
				unsigned long line_length = 0;
				const unsigned long disposition_length = ((field.filename ? (54 + strlen(field.filename)) : 41) + strlen(field.name));
				unsigned long field_length = (4 + boundary_length + disposition_length);

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

			sprintf(request_message_information, (
				"%s %s HTTP/1.1\r\n"
				"Host: %s:%d\r\n"
				"Accept: */*\r\n"
				"Connection: close\r\n"
				"Content-Type: multipart/form-data; boundary=%s\r\n"
				"Content-Length: %ld\r\n"
				"%s\r\n"
			), config.method, url.path, url.hostname, url.port, boundary, data_length, header_text);

			request_message_length = strlen(request_message_information);
			request_message = allocate(NULL, 0, request_message_length + data_length + 1, sizeof(char));

			strcpy(request_message, request_message_information);
			memcpy(request_message + request_message_length, data, data_length);
			request_message_length += data_length;

			free(data);
		} else if (config.body.payload.data != NULL) {
			const char *body = config.body.payload.data;
			const unsigned long body_length = strlen(body);

			sprintf(request_message_information, (
				"%s %s HTTP/1.1\r\n"
				"Host: %s:%d\r\n"
				"Accept: */*\r\n"
				"Connection: close\r\n"
				"Content-Length: %ld\n"
				"%s\r\n"
			), config.method, url.path, url.hostname, url.port, body_length, header_text);

			request_message_length = strlen(request_message_information);
			request_message = allocate(NULL, 0, request_message_length + body_length + 1, sizeof(char));

			strcpy(request_message, request_message_information);
			memcpy(request_message + request_message_length, body, body_length);
			request_message_length += body_length;
		} else {
			sprintf(request_message_information,
				"%s %s HTTP/1.1\r\n"
				"Host: %s:%d\r\n"
				"Accept: */*\r\n"
				"Connection: close\r\n"
				"%s\r\n"
			, config.method, url.path, url.hostname, url.port, header_text);

			request_message_length = strlen(request_message_information);
			request_message = allocate(NULL, 0, request_message_length + 1, sizeof(char));
			strcpy(request_message, request_message_information);
		}

		free_url(url);

		if (s_write(ssl, sockfd, request_message, request_message_length) <= 0) {
			close_socket(sockfd, ssl);
			throw_network("write()", tls);
		}

		free(request_message);

		unsigned long read_size = 0;
		unsigned long response_message_length = 0;

		while ((read_size = s_read(ssl, sockfd, buffer, HTTP_BUFFER_SIZE)) > 0) {
			if (errno != 0) {
				close_socket(sockfd, ssl);
				throw_network("read()", tls);
			} else {
				response_message = allocate(response_message, response_message_length + 1, response_message_length + read_size + 1, sizeof(char));
				memcpy(response_message + response_message_length, buffer, read_size);
				response_message_length += read_size;
			}
		}

		struct Split line_splitter = split(response_message, response_message_length, "\r\n");
		struct Split status_splitter = split(line_splitter.data[0].data, line_splitter.data[0].length, " ");

		const unsigned long status_message_length = calculate_join((struct Join *) status_splitter.data + 2, status_splitter.size - 2, " ");

		response.status.code = atoi(status_splitter.data[1].data);
		response.status.message = allocate(NULL, -1, status_message_length + 1, sizeof(char));
		join((struct Join *) status_splitter.data + 2, response.status.message, status_splitter.size - 2, " ");

		split_free(status_splitter);

		unsigned long i = 0;

		for (i = 1; i < line_splitter.size; ++i) {
			if (line_splitter.data[i].data[0] == 0) {
				break;
			} else {
				const char *value = strstr(line_splitter.data[i].data, ": ") + 2;
				const unsigned long value_length = strlen(value);
				const unsigned long name_length = (line_splitter.data[i].length - value_length - 2);

				response.headers = allocate(response.headers, -1, i, sizeof(struct Header));
				struct Header *header = &response.headers[i - 1];

				header->name = allocate(NULL, -1, name_length + 1, sizeof(char));
				header->value = allocate(NULL, -1, value_length + 1, sizeof(char));
				strncpy(header->name, line_splitter.data[i].data, name_length);
				strncpy(header->value, value, value_length);
				++response.header_size;
			}
		}

		const char *transfer_encoding_value = get_header(response.headers, response.header_size, "transfer-encoding").value;

		if (transfer_encoding_value != NULL && strcmp(transfer_encoding_value, "chunked") == 0) {
			++i;

			while (i < line_splitter.size) {
				unsigned long hex_length = ahtoi(line_splitter.data[i].data);
				const unsigned long line_length = line_splitter.data[i + 1].length;

				if (line_length != 0) {
					if (line_length == hex_length) {
						response.data_size += hex_length;
						response.data = allocate(response.data, -1, response.data_size + 1, sizeof(char));
						memcpy(response.data + response.data_size - hex_length, line_splitter.data[i + 1].data, line_length);
					} else {
						throw_network("invalid http message encoding", false);
					}
				} else {
					break;
				}

				i += 2;
			}

			response.data[response.data_size] = 0;
		} else if (response.status.code != 204) {
			response.data_size = atoi(get_header(response.headers, response.header_size, "content-length").value);
			response.data = allocate(response.data, -1, response.data_size + 1, sizeof(char));

			join((struct Join *) line_splitter.data + i + 1, (char *) response.data, line_splitter.size - i - 1, "\r\n");
		}

		split_free(line_splitter);
		free(response_message);

		close_socket(sockfd, ssl);
	} else {
		close_socket(sockfd, ssl);
		throw_network("connect()", tls);
	}

	return response;
}
