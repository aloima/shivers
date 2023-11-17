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

	struct FormDataField {
		char *name;
		char *data;
		size_t data_size;
		struct Header *headers;
		size_t header_size;
		char *filename;
	};

	struct FormData {
		struct FormDataField *fields;
		size_t field_size;
		char *boundary;
	};

	struct RequestBody {
		union {
			char *data;
			struct FormData formdata;
		} payload;

		bool is_formdata;
	};

	struct RequestConfig {
		char *url;
		char *method;
		struct Header *headers;
		size_t header_size;
		struct RequestBody body;
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
	struct Header get_header(struct Header *headers, const size_t header_size, const char *name);
	struct hostent *resolve_hostname(char *hostname);
	void close_socket(int sockfd, SSL *ssl);
	void add_field_to_formdata(struct FormData *formdata, const char *name, const char *data, const size_t data_size, const char *filename);
	void add_header_to_formdata_field(struct FormData *formdata, const char *field_name, const char *header_name, const char *header_value);
	void free_formdata(struct FormData formdata);

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
