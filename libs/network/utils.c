#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>

#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <netdb.h>

#include <openssl/ssl.h>
#include <openssl/err.h>

#include <network.h>

URL parse_url(const char *data) {
	URL url;
	memset(&url, 0, sizeof(URL));

	Split splitter = split((char *) data, "/");
	Split hostname_splitter = split(splitter.data[2], ":");

	size_t protocol_length = strlen(splitter.data[0]);
	url.protocol = allocate(NULL, 0, protocol_length, sizeof(char));
	strncpy(url.protocol, splitter.data[0], protocol_length - 1);

	url.hostname = allocate(NULL, 0, strlen(hostname_splitter.data[0]) + 1, sizeof(char));
	strcpy(url.hostname, hostname_splitter.data[0]);

	if (hostname_splitter.size == 2) {
		url.port = (unsigned short) atoi(hostname_splitter.data[1]);
	} else {
		if (strcmp(url.protocol, "https") == 0 || strcmp(url.protocol, "wss") == 0) {
			url.port = 443;
		} else if (strcmp(url.protocol, "http") == 0 || strcmp(url.protocol, "ws") == 0) {
			url.port = 80;
		}
	}

	url.path = allocate(NULL, 0, calculate_join(splitter.data + 3, splitter.size - 3, "/") + 2, sizeof(char));
	url.path[0] = '/';
	join(splitter.data + 3, url.path + 1, splitter.size - 3, "/");

	split_free(&splitter);
	split_free(&hostname_splitter);

	return url;
}

void free_url(URL url) {
	free(url.protocol);
	free(url.hostname);
	free(url.path);
	memset(&url, 0, sizeof(URL));
}

struct hostent *resolve_hostname(char *hostname) {
	struct hostent *result = gethostbyname(hostname);

	if (result == NULL && strncmp(hostname, "www.", 4) == 0) {
		result = gethostbyname(hostname + 4);
	}

	return result;
}

void close_socket(int sockfd, SSL *ssl) {
	if (ssl != NULL) {
		SSL_shutdown(ssl);
		SSL_CTX_free(SSL_get_SSL_CTX(ssl));
		SSL_free(ssl);
	}

	close(sockfd);
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

unsigned long combine_bytes(unsigned char *bytes, size_t byte_count) {
	unsigned long result = 0;

	for (int i = 0; i < byte_count; ++i) {
		result |= (bytes[i] << ((byte_count - i - 1) * 8));
	}

	return result;
}

Header get_header(Header *headers, size_t header_size, char *name) {
	Header header;
	memset(&header, 0, sizeof(header));

	char header_name[1024] = {0};
	strtolower(header_name, name);

	for (size_t i = 0; i < header_size; ++i) {
		char current_name[1024] = {0};
		strtolower(current_name, headers[i].name);

		if (strcmp(header_name, current_name) == 0) {
			header = headers[i];
			break;
		}
	}

	return header;
}

size_t _read(SSL *ssl, int sockfd, char *buffer, size_t size) {
	if (ssl != NULL) {
		return SSL_read(ssl, buffer, size);
	} else {
		return read(sockfd, buffer, size);
	}
}

size_t _write(SSL *ssl, int sockfd, char *buffer, size_t size) {
	size_t result;
	bool err = false;

	if (ssl != NULL) {
		result = SSL_write(ssl, buffer, size);
		err = (result <= 0);
	} else {
		result = write(sockfd, buffer, size);
		err = (result < 0);
	}

	if (err) {
		throw_network("_write()", !!ssl);
	}

	return result;
}
