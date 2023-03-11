#include <utils.h>

#ifndef JSON_H_
	#define JSON_H_

	#define JSON_UNSPECIFIED 0
	#define JSON_BOOLEAN 1
	#define JSON_NUMBER 2
	#define JSON_STRING 3
	#define JSON_OBJECT 4
	#define JSON_ARRAY 5
	#define JSON_NULL 6

	typedef union {
		char type;
		char *string;
		long number;
		bool boolean;
	} __attribute__((packed)) JSONValue;

	typedef struct {
		char type;
		char *key;
		void *value;
		void *parent;
		size_t size;
	} __attribute__((packed)) JSONElement;

	JSONElement *json_parse(char *text);
	char *json_stringify(JSONElement *element);
	JSONValue json_get_val(JSONElement *element, char *search);
	void json_free(JSONElement *element);
#endif
