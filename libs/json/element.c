#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <utils.h>
#include <json.h>

jsonelement_t *create_empty_json_element(const bool is_array) {
	jsonelement_t *element = allocate(NULL, 0, 1, sizeof(jsonelement_t));
	element->type = (is_array ? JSON_ARRAY : JSON_OBJECT);

	return element;
}

jsonelement_t *clone_json_element(jsonelement_t *element) {
	jsonelement_t *result = allocate(NULL, -1, 1, sizeof(jsonelement_t));
	result->type = element->type;

	if (element->key != NULL) {
		const unsigned long long key_size = (strlen(element->key) + 1);
		result->key = allocate(NULL, -1, key_size, sizeof(char));
		memcpy(result->key, element->key, key_size);
	}

	if (element->type == JSON_ARRAY || element->type == JSON_OBJECT) {
		const unsigned int size = result->size = element->size;
		result->value = allocate(NULL, -1, size, sizeof(jsonelement_t));

		for (unsigned int i = 0; i < size; ++i) {
			((jsonelement_t **) result->value)[i] = clone_json_element(((jsonelement_t **) element->value)[i]);
		}
	} else {
		switch (result->type) {
			case JSON_NUMBER:
				result->value = allocate(NULL, -1, 1, sizeof(double));
				((double *) result->value)[0] = *((double *) element->value);
				break;

			case JSON_STRING:
				result->size = element->size;
				result->value = allocate(NULL, -1, result->size + 1, sizeof(char));
				memcpy(result->value, element->value, result->size + 1);
				break;

			case JSON_BOOLEAN:
				result->value = allocate(NULL, -1, 1, sizeof(bool));
				((bool *) result->value)[0] = *((bool *) element->value);
				break;

			default:
				break;
		}
	}

	return result;
}
