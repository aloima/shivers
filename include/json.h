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
		size_t size;
	};

	typedef struct JSONElement jsonelement_t;

	typedef union {
		char *string;
		float number;
		bool boolean;
		struct JSONElement *object;
		struct JSONElement *array;
	} jsonvalue_t;

	typedef struct {
		jsonvalue_t value;
		bool exist;
		char type;
	} jsonresult_t;

	jsonelement_t *json_parse(const char *text);
	void json_free(jsonelement_t *element);

	char *json_stringify(jsonelement_t *element);

	jsonresult_t json_get_val(jsonelement_t *element, const char *search);

	jsonelement_t *create_empty_json_element(bool is_array);
	void add_json_element(jsonelement_t **object, const char *key, void *data, const char type);
#endif
