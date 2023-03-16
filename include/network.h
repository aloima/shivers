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

	void response_free(Response *response);

	Response request(RequestConfig config);
#endif
