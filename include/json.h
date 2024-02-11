#include <stdbool.h>

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

	struct JSONElement {
		char type;
		char *key;
		void *value;
		struct JSONElement *parent;
		unsigned long size;
	};

	typedef struct JSONElement jsonelement_t;

	typedef union {
		char *string;
		double number;
		bool boolean;
		jsonelement_t *object;
		jsonelement_t *array;
	} jsonvalue_t;

	typedef struct {
		jsonvalue_t value;
		jsonelement_t *element;
		bool exist;
		char type;
	} jsonresult_t;

	jsonelement_t *json_parse(const char *text);
	void json_free(jsonelement_t *element, const bool all);

	char *json_stringify(const jsonelement_t *element, const unsigned char fractional_limit);

	jsonresult_t json_get_val(jsonelement_t *element, const char *search);

	jsonelement_t *create_empty_json_element(const bool is_array);
	void json_set_val(jsonelement_t *object, const char *key, void *data, const char type);
#endif
