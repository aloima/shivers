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
	url.protocol = allocate(NULL, protocol_length, sizeof(char));
	strncpy(url.protocol, splitter.data[0], protocol_length - 1);

	url.hostname = allocate(NULL, strlen(hostname_splitter.data[0]) + 1, sizeof(char));
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

	url.path = allocate(NULL, calculate_join(splitter.data + 3, splitter.size - 3, "/") + 2, sizeof(char));
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

void throw(const char *value, bool tls) {
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

bool set_nonblocking(int sockfd) {
	int flags = fcntl(sockfd, F_GETFL, 0);

	if (flags == -1) {
		return false;
	}

	return (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1);
}
