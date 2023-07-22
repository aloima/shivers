#include <math.h>
#include <stdio.h>

#include <json.h>

JSONElement *create_empty_json_object() {
	JSONElement *object = allocate(NULL, 1, sizeof(JSONElement));
	object->type = JSON_OBJECT;

	return object;
}

void add_json_element(JSONElement **object, const char *key, void *data, const char type) {
	size_t key_length = strlen(key);

	++(*object)->size;

	(*object)->value = allocate((*object)->value, (*object)->size, sizeof(JSONElement));
	((JSONElement **) (*object)->value)[(*object)->size - 1] = allocate(NULL, 1, sizeof(JSONElement));
	((JSONElement **) (*object)->value)[(*object)->size - 1]->type = type;
	((JSONElement **) (*object)->value)[(*object)->size - 1]->key = allocate(NULL, key_length + 1, sizeof(JSONElement));

	memcpy(((JSONElement **) (*object)->value)[(*object)->size - 1]->key, key, key_length);

	if (type == JSON_STRING) {
		size_t data_length = strlen(data);

		((JSONElement **) (*object)->value)[(*object)->size - 1]->value = allocate(NULL, data_length + 1, sizeof(char));
		memcpy(((JSONElement **) (*object)->value)[(*object)->size - 1]->value, data, data_length);
	} else if (type == JSON_BOOLEAN) {
		((JSONElement **) (*object)->value)[(*object)->size - 1]->value = allocate(NULL, 1, sizeof(char));
		((bool *) ((JSONElement **) (*object)->value)[(*object)->size - 1]->value)[0] = ((bool *) data)[0];
	} else if (type == JSON_NUMBER) {
		long number = ((long *) data)[0];
		/*bool has_fractional = (roundf(data) == ((float *) data)[0]);

		if (has_fractional) {
			printf("%ld\n", number);
			float fractional = fmod(((float *) data)[0], 1.0);
			((JSONElement **) (*object)->value)[(*object)->size - 1]->size = 2;

			((JSONElement **) (*object)->value)[(*object)->size - 1]->value = allocate(NULL, 2, sizeof(long));
			memcpy(((JSONElement **) (*object)->value)[(*object)->size - 1]->value, &number, sizeof(long));
			memcpy(((JSONElement **) (*object)->value)[(*object)->size - 1]->value + 1, &fractional, sizeof(long));
		} else */{
			((JSONElement **) (*object)->value)[(*object)->size - 1]->size = 1;

			((JSONElement **) (*object)->value)[(*object)->size - 1]->value = allocate(NULL, 1, sizeof(long));
			memcpy(((JSONElement **) (*object)->value)[(*object)->size - 1]->value, &number, sizeof(long));
		}
	}
}