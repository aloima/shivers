#include <utils.h>

#ifndef NETWORK_H_
	#define NETWORK_H_

	typedef struct {
		char *name;
		char *value;
	} __attribute__((packed)) Header;

	typedef struct {
		char *url;
		char *method;
		unsigned short port;
		Header *headers;
		size_t header_size;
	} __attribute__((packed)) RequestConfig;

	typedef struct {
		bool success;

		struct {
			short code;
			char *message;
		} __attribute__((packed)) status;

		char *data;
		Header *headers;
		size_t header_size;
	} __attribute__((packed)) Response;

	typedef struct {
		char *url;
		unsigned short port;
		void (*onstart)(void);
		void (*onclose)(void);
		void (*onmessage)(char *);
	} __attribute__((packed)) Websocket;

	void response_free(Response *response);
	Response request(RequestConfig config);

	void connect_websocket(Websocket *websocket);
#endif
