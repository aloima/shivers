#include <math.h>
#include <stdio.h>

#include <json.h>

jsonelement_t *create_empty_json_element(bool is_array) {
	jsonelement_t *element = allocate(NULL, 0, 1, sizeof(jsonelement_t));

	if (is_array) {
		element->type = JSON_ARRAY;
	} else {
		element->type = JSON_OBJECT;
	}

	return element;
}

void add_json_element(jsonelement_t **target, const char *key, void *data, const char type) {
	++(*target)->size;

	(*target)->value = allocate((*target)->value, (*target)->size - 1, (*target)->size, sizeof(jsonelement_t));
	((jsonelement_t **) (*target)->value)[(*target)->size - 1] = allocate(NULL, 0, 1, sizeof(jsonelement_t));
	((jsonelement_t **) (*target)->value)[(*target)->size - 1]->type = type;

	if (key != NULL) {
		size_t key_length = strlen(key);
		((jsonelement_t **) (*target)->value)[(*target)->size - 1]->key = allocate(NULL, 0, key_length + 1, sizeof(jsonelement_t));
		memcpy(((jsonelement_t **) (*target)->value)[(*target)->size - 1]->key, key, key_length);
	}

	if (type == JSON_STRING) {
		size_t data_length = strlen(data);

		((jsonelement_t **) (*target)->value)[(*target)->size - 1]->value = allocate(NULL, 0, data_length + 1, sizeof(char));
		memcpy(((jsonelement_t **) (*target)->value)[(*target)->size - 1]->value, data, data_length);
	} else if (type == JSON_BOOLEAN) {
		((jsonelement_t **) (*target)->value)[(*target)->size - 1]->value = allocate(NULL, 0, 1, sizeof(char));
		((bool *) ((jsonelement_t **) (*target)->value)[(*target)->size - 1]->value)[0] = ((bool *) data)[0];
	} else if (type == JSON_NUMBER) {
		long number = ((long *) data)[0];

		// TODO: float support
		((jsonelement_t **) (*target)->value)[(*target)->size - 1]->size = 1;

		((jsonelement_t **) (*target)->value)[(*target)->size - 1]->value = allocate(NULL, 0, 1, sizeof(long));
		memcpy(((jsonelement_t **) (*target)->value)[(*target)->size - 1]->value, &number, sizeof(long));
	} else if (type == JSON_OBJECT || type == JSON_ARRAY) {
		size_t size = ((jsonelement_t *) data)->size;
		jsonelement_t *target_element = ((jsonelement_t **) (*target)->value)[(*target)->size - 1];

		for (size_t i = 0; i < size; ++i) {
			jsonelement_t *sub_element = ((jsonelement_t **) ((jsonelement_t *) data)->value)[i];

			if (sub_element->type == JSON_OBJECT || sub_element->type == JSON_ARRAY) {
				add_json_element(&target_element, sub_element->key, sub_element, sub_element->type);
			} else {
				add_json_element(&target_element, sub_element->key, sub_element->value, sub_element->type);
			}
		}
	}
}
