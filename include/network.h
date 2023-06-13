#include <stdbool.h>

#include <openssl/ssl.h>

#include <utils.h>

#ifndef NETWORK_H_
	#define NETWORK_H_

	typedef struct {
		char *name;
		char *value;
	} Header;

	typedef struct {
		char *url;
		char *method;
		unsigned short port;
		Header *headers;
		size_t header_size;
	} RequestConfig;

	typedef struct {
		bool success;

		struct {
			short code;
			char *message;
		} status;

		char *data;
		Header *headers;
		size_t header_size;
	} Response;

	typedef struct {
		int sockfd;
		SSL *ssl;

		char *url;
		unsigned short port;

		void (*onstart)(void);
		void (*onclose)(void);
		void (*onmessage)(const char *message);
	} Websocket;

	void response_free(Response *response);
	Response request(RequestConfig config);

	struct hostent *resolve_hostname(char *hostname);
	void close_socket(int sockfd, SSL *ssl);
	void throw(const char *value, bool tls);

	void connect_websocket(Websocket *websocket);
	void send_websocket_message(Websocket *websocket, const char *message);
	void *message_thread(void *ptr);
#endif
