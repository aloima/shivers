#include <stdbool.h>

#include <openssl/ssl.h>

#include <utils.h>

#ifndef NETWORK_H_
	#define NETWORK_H_

	typedef struct {
		char *protocol;
		char *hostname;
		char *path;
		unsigned short port;
	} URL;

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

	Response request(RequestConfig config);
	void response_free(Response *response);

	void throw(const char *value, bool tls);

	URL parse_url(const char *data);
	void free_url(URL url);

	bool set_nonblocking(int sockfd);
	struct hostent *resolve_hostname(char *hostname);
	void close_socket(int sockfd, SSL *ssl);

	// Websocket Headers
	typedef struct {
		bool fin, rsv[3], mask;
		unsigned char opcode;
		char *payload;
		size_t payload_length;
	} WebsocketFrame;

	typedef struct {
		void (*onstart)(void);
		void (*onmessage)(const WebsocketFrame frame);
		void (*onclose)(void);
	} WebsocketMethods;

	typedef struct {
		int sockfd;
		int epollfd;
		SSL *ssl;
		URL url;
		WebsocketMethods methods;
		bool connected;
	} Websocket;

	Websocket create_websocket(const char *url, const WebsocketMethods methods);
	void send_websocket_message(Websocket *websocket, const char *message);
#endif
