#include <math.h>
#include <stdio.h>

#include <json.h>

JSONElement *create_empty_json_element(bool is_array) {
	JSONElement *element = allocate(NULL, 0, 1, sizeof(JSONElement));

	if (is_array) {
		element->type = JSON_ARRAY;
	} else {
		element->type = JSON_OBJECT;
	}

	return element;
}

void add_json_element(JSONElement **target, const char *key, void *data, const char type) {
	++(*target)->size;

	(*target)->value = allocate((*target)->value, (*target)->size - 1, (*target)->size, sizeof(JSONElement));
	((JSONElement **) (*target)->value)[(*target)->size - 1] = allocate(NULL, 0, 1, sizeof(JSONElement));
	((JSONElement **) (*target)->value)[(*target)->size - 1]->type = type;

	if (key != NULL) {
		size_t key_length = strlen(key);
		((JSONElement **) (*target)->value)[(*target)->size - 1]->key = allocate(NULL, 0, key_length + 1, sizeof(JSONElement));
		memcpy(((JSONElement **) (*target)->value)[(*target)->size - 1]->key, key, key_length);
	}

	if (type == JSON_STRING) {
		size_t data_length = strlen(data);

		((JSONElement **) (*target)->value)[(*target)->size - 1]->value = allocate(NULL, 0, data_length + 1, sizeof(char));
		memcpy(((JSONElement **) (*target)->value)[(*target)->size - 1]->value, data, data_length);
	} else if (type == JSON_BOOLEAN) {
		((JSONElement **) (*target)->value)[(*target)->size - 1]->value = allocate(NULL, 0, 1, sizeof(char));
		((bool *) ((JSONElement **) (*target)->value)[(*target)->size - 1]->value)[0] = ((bool *) data)[0];
	} else if (type == JSON_NUMBER) {
		long number = ((long *) data)[0];

		// TODO: float support
		((JSONElement **) (*target)->value)[(*target)->size - 1]->size = 1;

		((JSONElement **) (*target)->value)[(*target)->size - 1]->value = allocate(NULL, 0, 1, sizeof(long));
		memcpy(((JSONElement **) (*target)->value)[(*target)->size - 1]->value, &number, sizeof(long));
	} else if (type == JSON_OBJECT || type == JSON_ARRAY) {
		size_t size = ((JSONElement *) data)->size;
		JSONElement *target_element = ((JSONElement **) (*target)->value)[(*target)->size - 1];

		for (size_t i = 0; i < size; ++i) {
			JSONElement *sub_element = ((JSONElement **) ((JSONElement *) data)->value)[i];

			if (sub_element->type == JSON_OBJECT || sub_element->type == JSON_ARRAY) {
				add_json_element(&target_element, sub_element->key, sub_element, sub_element->type);
			} else {
				add_json_element(&target_element, sub_element->key, sub_element->value, sub_element->type);
			}
		}
	}
}
