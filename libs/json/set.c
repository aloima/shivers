#include <math.h>
#include <stdio.h>
#include <string.h>

#include <json.h>

jsonelement_t *create_empty_json_element(const bool is_array) {
	jsonelement_t *element = allocate(NULL, 0, 1, sizeof(jsonelement_t));

	if (is_array) {
		element->type = JSON_ARRAY;
	} else {
		element->type = JSON_OBJECT;
	}

	return element;
}

void json_set_val(jsonelement_t *target, const char *key, void *data, const char type) {
	jsonresult_t result = {0};

	if (key != NULL) {
		result = json_get_val(target, key);
	}

	if (result.exist) {
		result.element->type = type;

		if (type == JSON_NUMBER) {
			const long number = ((long *) data)[0];

			result.element->size = 1;
			result.element->value = allocate(result.element->value, -1, 1, sizeof(long));
			memcpy(result.element->value, &number, sizeof(long));
		} else if (type == JSON_STRING) {
			const size_t data_length = strlen(data);

			result.element->value = allocate(result.element->value, -1, data_length + 1, sizeof(char));
			strcpy(result.element->value, data);
		} else if (type == JSON_BOOLEAN) {
			result.element->value = allocate(result.element->value, -1, 1, sizeof(char));
			((bool *) result.element->value)[0] = ((bool *) data)[0];
		}
	} else {
		++target->size;

		target->value = allocate(target->value, -1, target->size, sizeof(jsonelement_t *));
		((jsonelement_t **) target->value)[target->size - 1] = allocate(NULL, 0, 1, sizeof(jsonelement_t));
		((jsonelement_t **) target->value)[target->size - 1]->type = type;

		if (key != NULL) {
			size_t key_length = strlen(key);
			((jsonelement_t **) target->value)[target->size - 1]->key = allocate(NULL, -1, key_length + 1, sizeof(jsonelement_t));
			strncpy(((jsonelement_t **) target->value)[target->size - 1]->key, key, key_length);
		}

		if (type == JSON_STRING) {
			const size_t data_length = strlen(data);

			((jsonelement_t **) target->value)[target->size - 1]->value = allocate(NULL, -1, data_length + 1, sizeof(char));
			strcpy(((jsonelement_t **) target->value)[target->size - 1]->value, data);
		} else if (type == JSON_BOOLEAN) {
			((jsonelement_t **) target->value)[target->size - 1]->value = allocate(NULL, -1, 1, sizeof(char));
			((bool *) ((jsonelement_t **) target->value)[target->size - 1]->value)[0] = ((bool *) data)[0];
		} else if (type == JSON_NUMBER) {
			const long number = ((long *) data)[0];

			// TODO: float support
			((jsonelement_t **) target->value)[target->size - 1]->size = 1;

			((jsonelement_t **) target->value)[target->size - 1]->value = allocate(NULL, -1, 1, sizeof(long));
			memcpy(((jsonelement_t **) target->value)[target->size - 1]->value, &number, sizeof(long));
		} else if (type == JSON_OBJECT || type == JSON_ARRAY) {
			const size_t size = ((jsonelement_t *) data)->size;
			jsonelement_t *target_element = ((jsonelement_t **) target->value)[target->size - 1];

			for (size_t i = 0; i < size; ++i) {
				jsonelement_t *sub_element = ((jsonelement_t **) ((jsonelement_t *) data)->value)[i];

				if (sub_element->type == JSON_OBJECT || sub_element->type == JSON_ARRAY) {
					json_set_val(target_element, sub_element->key, sub_element, sub_element->type);
				} else {
					json_set_val(target_element, sub_element->key, sub_element->value, sub_element->type);
				}
			}
		}
	}
}
