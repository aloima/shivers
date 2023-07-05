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

	unsigned long combine_bytes(unsigned char *bytes, size_t byte_count);
	struct hostent *resolve_hostname(char *hostname);
	void close_socket(int sockfd, SSL *ssl);

	// Websocket Headers
	#define WEBSOCKET_FRAME_MAGIC 0x81;

	typedef struct {
		bool fin, rsv[3], mask;
		unsigned char opcode;
		char *payload;
		size_t payload_length;
	} WebsocketFrame;

	typedef struct {
		char *data;
		size_t size;
	} WebsocketTBS;

	typedef struct {
		void (*onstart)(void);
		void (*onmessage)(const WebsocketFrame frame);
		void (*onclose)(const short code, const char *reason);
	} WebsocketMethods;

	typedef struct {
		int sockfd;
		int epollfd;
		SSL *ssl;
		URL url;
		WebsocketMethods methods;
		bool connected;
		char *key;

		WebsocketTBS *tbs; // to be sent
		size_t tbs_size;
	} Websocket;

	Websocket create_websocket(const char *url, const WebsocketMethods methods);
	void connect_websocket(Websocket *websocket);
	void send_websocket_message(Websocket *websocket, const char *message);
#endif
