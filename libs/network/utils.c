#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>

#if defined(_WIN32)
	#include <winsock2.h>
#elif defined(__linux__)
	#include <sys/socket.h>
	#include <fcntl.h>
	#include <unistd.h>
	#include <netdb.h>
#endif

#include <openssl/ssl.h>
#include <openssl/err.h>

#include <network.h>
#include <utils.h>

struct URL parse_url(const char *data) {
	struct URL url = {0};
	struct Split splitter = split(data, strlen(data), "/");

	if (splitter.size < 3) {
		throw("request(): invalid url format");
	}

	struct Split hostname_splitter = split(splitter.data[2].data, splitter.data[2].length, ":");

	const unsigned long protocol_length = splitter.data[0].length;
	url.protocol = allocate(NULL, -1, protocol_length, sizeof(char));
	strncpy(url.protocol, splitter.data[0].data, protocol_length - 1);

	url.hostname = allocate(NULL, -1, hostname_splitter.data[0].length + 1, sizeof(char));
	strcpy(url.hostname, hostname_splitter.data[0].data);

	if (hostname_splitter.size == 2) {
		url.port = atoi_s(hostname_splitter.data[1].data, hostname_splitter.data[1].length);
	} else {
		if (strsame(url.protocol, "https") || strsame(url.protocol, "wss")) {
			url.port = 443;
		} else if (strsame(url.protocol, "http") || strsame(url.protocol, "ws")) {
			url.port = 80;
		}
	}

	if (splitter.size == 3) {
		url.path = allocate(NULL, 0, 2, sizeof(char));
		url.path[0] = '/';
	} else {
		const unsigned long join_length = calculate_join((struct Join *) (splitter.data + 3), splitter.size - 3, "/");
		url.path = allocate(NULL, 0, join_length + 2, sizeof(char));
		url.path[0] = '/';
		join((struct Join *) (splitter.data + 3), url.path + 1, splitter.size - 3, "/");
	}

	split_free(splitter);
	split_free(hostname_splitter);

	return url;
}

void free_url(struct URL url) {
	free(url.protocol);
	free(url.hostname);
	free(url.path);
}

struct hostent *resolve_hostname(char *hostname) {
	struct hostent *result = gethostbyname(hostname);

	if (result == NULL && strncmp(hostname, "www.", 4) == 0) {
		result = gethostbyname(hostname + 4);
	}

	return result;
}

void throw_network(const char *value, bool tls) {
	unsigned long tls_error;

	if (errno != 0) {
		perror(value);
		exit(EXIT_FAILURE);
	} else if (tls && (tls_error = ERR_get_error()) != 0) {
		char message[1024];

		ERR_error_string(tls_error, message);
		fprintf(stderr, "%s: %s\n", value, message);
		exit(EXIT_FAILURE);
	} else {
		fprintf(stderr, "%s\n", value);
		exit(EXIT_FAILURE);
	}
}

unsigned long combine_bytes(unsigned char *bytes, unsigned long byte_count) {
	unsigned long result = 0;

	for (unsigned char i = 0; i < byte_count; ++i) {
		result |= (bytes[i] << ((byte_count - i - 1) * 8));
	}

	return result;
}

struct Header get_header(struct Header *headers, const unsigned long header_size, const char *name) {
	struct Header header = {0};

	char header_name[1024];
	strtolower(header_name, name);

	for (unsigned long i = 0; i < header_size; ++i) {
		char current_name[1024];
		strtolower(current_name, headers[i].name);

		if (strsame(header_name, current_name)) {
			header = headers[i];
			break;
		}
	}

	return header;
}

#if defined(_WIN32)
unsigned long s_read(SSL *ssl, SOCKET sockfd, char *buffer, unsigned long size) {
#elif defined(__linux__)
unsigned long s_read(SSL *ssl, int sockfd, char *buffer, unsigned long size) {
#endif

	if (ssl != NULL) {
		return SSL_read(ssl, buffer, size);
	} else {
		return recv(sockfd, buffer, size, 0);
	}
}

#if defined(_WIN32)
unsigned long s_write(SSL *ssl, SOCKET sockfd, char *buffer, unsigned long size) {
#elif defined(__linux)
unsigned long s_write(SSL *ssl, int sockfd, char *buffer, unsigned long size) {
#endif

	unsigned long result;
	bool err = false;

	if (ssl != NULL) {
		result = SSL_write(ssl, buffer, size);
		err = (result <= 0);
	} else {
		result = send(sockfd, buffer, size, 0);
		err = (result < 0);
	}

	if (err) {
		throw_network("s_write()", !!ssl);
	}

	return result;
}

#if defined(_WIN32)
void close_socket(SOCKET sockfd, SSL *ssl) {
#elif defined(__linux)
void close_socket(int sockfd, SSL *ssl) {
#endif

	if (ssl != NULL) {
		SSL_shutdown(ssl);
		SSL_CTX_free(SSL_get_SSL_CTX(ssl));
		SSL_free(ssl);
	}

	#if defined(_WIN32)
		closesocket(sockfd);
	#elif defined(__linux__)
		close(sockfd);
	#endif
}

char *percent_encode(const char *data) {
	const char reserved_chars[16] = {
		' ', '[', ']', '@',
		'!', '$', '\'', '(',
		')', '*', '+', ',', ';',
		'\0'
	}; // Some of reserved characters are not added because of syntax of URL.
	const unsigned long length = strlen(data);
	unsigned long result_length = (length + 1);
	char *result = allocate(NULL, -1, (length + 1), sizeof(char));

	for (unsigned long i = 0; i < length; ++i) {
		const char ch = data[i];

		if (char_at(reserved_chars, data[i]) != -1) {
			result = allocate(result, -1, result_length + 2, sizeof(char));
			result_length += 2;

			char hex[4];
			sprintf(hex, "%%%x", ch);
			strncat(result, hex, 4);
		} else {
			strncat(result, &ch, 1);
		}
	}

	return result;
}

void add_field_to_formdata(struct FormData *formdata, const char *name, const char *data, const unsigned long long data_size, const char *filename) {
	++formdata->field_size;
	formdata->fields = allocate(formdata->fields, -1, formdata->field_size, sizeof(struct FormDataField));

	const unsigned int field_name_size = (strlen(name) + 1);
	char *field_name = allocate(NULL, -1, field_name_size, sizeof(char));
	memcpy(field_name, name, field_name_size);

	const unsigned long long field_data_size = ((data_size == -1) ? (strlen(data) + 1) : data_size);
	char *field_data = allocate(NULL, -1, field_data_size, sizeof(char));
	memcpy(field_data, data, field_data_size);

	char *field_filename = NULL;

	if (filename) {
		const unsigned int filename_size = (strlen(filename) + 1);
		field_filename = allocate(NULL, -1, filename_size, sizeof(char));
		memcpy(field_filename, filename, filename_size);
	}

	formdata->fields[formdata->field_size - 1] = (struct FormDataField) {
		.name = field_name,
		.data = field_data,
		.data_size = field_data_size,
		.filename = field_filename
	};
}

void add_header_to_formdata_field(struct FormData *formdata, const char *field_name, const char *header_name, const char *header_value) {
	const unsigned int field_size = formdata->field_size;

	for (unsigned int i = 0; i < field_size; ++i) {
		struct FormDataField *field = &(formdata->fields[i]);

		if (strsame(field->name, field_name)) {
			++field->header_size;
			field->headers = allocate(field->headers, -1, field->header_size, sizeof(struct Header));

			const unsigned int header_name_size = (strlen(header_name) + 1);
			char *name = allocate(NULL, -1, header_name_size, sizeof(char));
			memcpy(name, header_name, header_name_size);

			const unsigned int header_value_size = (strlen(header_value) + 1);
			char *value = allocate(NULL, -1, header_value_size, sizeof(char));
			memcpy(value, header_value, header_value_size);

			field->headers[field->header_size - 1] = (struct Header) {
				.name = name,
				.value = value
			};

			break;
		}
	}
}

void free_formdata(struct FormData formdata) {
	for (unsigned long i = 0; i < formdata.field_size; ++i) {
		struct FormDataField field = formdata.fields[i];

		if (field.header_size != 0) {
			for (unsigned long h = 0; h < field.header_size; ++h) {
				struct Header header = field.headers[h];
				free(header.name);
				free(header.value);
			}

			free(field.headers);
		}

		free(field.name);
		free(field.data);

		if (field.filename) {
			free(field.filename);
		}
	}

	free(formdata.fields);
}
