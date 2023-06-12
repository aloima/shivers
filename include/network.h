#include <stdbool.h>

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
		char *url;
		unsigned short port;
		void (*onstart)(void);
		void (*onclose)(void);
		void (*onmessage)(char *);
	} Websocket;

	void response_free(Response *response);
	Response request(RequestConfig config);

	void connect_websocket(Websocket *websocket);
#endif
