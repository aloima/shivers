#include <stdint.h>
#include <stdbool.h>

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
		char *string;
		intmax_t number;
		bool boolean;
		void **object;
		void **array;
	} JSONValue;

	typedef struct {
		char *key;
		void *value;
		void *parent;
		uint8_t type;
		size_t size;
	} JSONElement;

	JSONElement *json_parse(char *text);
	JSONElement *json_stringify(JSONElement *element);
	void json_free(JSONElement *element);
#endif
