#include <stdbool.h>

#include <openssl/ssl.h>

#include <utils.h>

#ifndef NETWORK_H_
	#define NETWORK_H_

	struct URL {
		char *protocol;
		char *hostname;
		char *path;
		unsigned short port;
	};

	struct Header {
		char *name;
		char *value;
	};

	struct RequestConfig {
		char *url;
		char *method;
		struct Header *headers;
		size_t header_size;
		char *body;
	};

	struct Response {
		bool success;

		struct {
			short code;
			char *message;
		} status;

		char *data;
		struct Header *headers;
		size_t header_size;
	};

	struct Response request(struct RequestConfig config);
	void response_free(struct Response *response);

	void throw_network(const char *value, bool tls);

	struct URL parse_url(const char *data);
	void free_url(struct URL url);
	char *percent_encode(const char *data);

	size_t _read(SSL *ssl, int sockfd, char *buffer, size_t size);
	size_t _write(SSL *ssl, int sockfd, char *buffer, size_t size);

	unsigned long combine_bytes(unsigned char *bytes, size_t byte_count);
	struct Header get_header(struct Header *headers, size_t header_size, char *name);
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
		struct URL url;
		WebsocketMethods methods;
		bool connected;
		bool closed;
		char *key;

		WebsocketTBS *tbs; // to be sent
		size_t tbs_size;
	} Websocket;

	Websocket create_websocket(const char *url, const WebsocketMethods methods);
	void connect_websocket(Websocket *websocket);
	void close_websocket(Websocket *websocket, const short code, const char *reason);
	void send_websocket_message(Websocket *websocket, const char *message);
#endif
