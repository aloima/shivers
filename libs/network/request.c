#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <network.h>
#include <utils.h>

static struct hostent *resolve_hostname(char *hostname) {
	struct hostent *result = gethostbyname(hostname);

	if (result == NULL && strncmp(hostname, "www.", 4) == 0) {
		result = gethostbyname(hostname + 4);
	}

	return result;
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

	Split splitter;
	char hostname[16384], path[49152], *_path = NULL;
	bool tls;
	short port;

	memset(&response, 0, sizeof(response));
	tls = (strncmp(config.url, "https", 5) == 0);
	port = (tls ? 443 : 80);

	splitter = split(config.url, "/");
	_path = join(splitter.data + 3, splitter.size - 3, "/");
	strcpy(hostname, splitter.data[2]);
	sprintf(path, "/%s", _path == NULL ? "" : _path);
	free(_path);
	split_free(&splitter);

	host = resolve_hostname(hostname);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	memcpy(&addr.sin_addr, host->h_addr_list[0], (size_t) host->h_length);

	if ((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		perror("socket()");
		return response;
	}

	if (connect(sockfd, (const struct sockaddr *) &addr, sizeof(addr)) == 0) {
		Split line_splitter, status_splitter, header_splitter;
		char buffer[1024], *response_message = NULL, request_message[262144];
		size_t response_message_length = 0, nread, i, response_data_length = 0;
		memset(&buffer, 0, 1024);

		sprintf(request_message,
			"GET %s HTTP/1.1\r\n"
			"Host: %s:%d\r\n"
			"Accept: */*\r\n"
			"Connection: close\r\n\r\n"
		, path, hostname, port);

		if (write(sockfd, request_message, strlen(request_message)) == -1) {
			perror("write()");
			return response;
		}

		while ((nread = (size_t) read(sockfd, buffer, 1023)) > 0) {
			if (errno != 0) {
				perror("read()");
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
			size_t line_length = strlen(line_splitter.data[i]);
			response_data_length += line_length;
			response.data = allocate(response.data, response_data_length + 1, sizeof(char));
			strncat(response.data, line_splitter.data[i], line_length);
		}

		split_free(&line_splitter);
		free(response_message);

		close(sockfd);
	} else {
		perror("connect()");
	}

	return response;
}
