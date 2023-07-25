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

	typedef struct {
		char type;
		char *key;
		void *value;
		void *parent;
		size_t size;
	} JSONElement;

	typedef union {
		char *string;
		float number;
		bool boolean;
		JSONElement *object;
		JSONElement *array;
	} JSONValue;

	typedef struct {
		JSONValue value;
		bool exist;
		char type;
	} JSONResult;

	JSONElement *json_parse(char *text);
	void json_free(JSONElement *element);

	char *json_stringify(JSONElement *element);

	JSONResult json_get_val(JSONElement *element, const char *search);

	JSONElement *create_empty_json_element(bool is_array);
	void add_json_element(JSONElement **object, const char *key, void *data, const char type); // use long for integers
#endif
