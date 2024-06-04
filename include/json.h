#include <stdbool.h>

#include <utils.h>

#ifndef JSON_H_
	#define JSON_H_

	enum JSONType {
		JSON_UNSPECIFIED,
		JSON_BOOLEAN,
		JSON_NUMBER,
		JSON_STRING,
		JSON_OBJECT,
		JSON_ARRAY,
		JSON_NULL
	};

	struct JSONElement {
		char *key;
		enum JSONType type;
		void *value;
		struct JSONElement *parent;
		unsigned int size;
	};

	typedef struct JSONElement jsonelement_t;

	typedef union {
		char *string;
		double number;
		bool boolean;
	} jsonvalue_t;

	typedef struct {
		jsonvalue_t value;
		jsonelement_t *element;
		bool exist;
	} jsonresult_t;

	jsonelement_t *json_parse(const char *text);
	void json_free(jsonelement_t *element, const bool all);

	jsonelement_t *create_empty_json_element(const bool is_array);
	jsonelement_t *clone_json_element(jsonelement_t *element);
	char *json_stringify(const jsonelement_t *element, const unsigned int fractional_limit);

	jsonresult_t json_get_val(jsonelement_t *element, const char *search);
	void json_del_val(jsonelement_t *element, const char *search);
	void json_set_val(jsonelement_t *object, const char *key, void *data, const enum JSONType type);
#endif
