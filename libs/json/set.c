#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include <utils.h>
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

// TODO: optimization for cpu instructions
static void set_value(jsonelement_t *element, void *value, const unsigned char type) {
	if (element->type == JSON_ARRAY || element->type == JSON_OBJECT) {
		for (size_t i = 0; i < element->size; ++i) {
			json_free(((jsonelement_t **) element->value)[i], false);
		}
	}

	element->type = type;

	if (type == JSON_STRING) {
		const size_t value_length = strlen(value);

		element->value = allocate(element->value, -1, value_length + 1, sizeof(char));
		strcpy(element->value, value);
	} else if (type == JSON_BOOLEAN) {
		element->value = allocate(element->value, -1, 1, sizeof(char));
		((bool *) element->value)[0] = ((bool *) value)[0];
	} else if (type == JSON_NUMBER) {
		element->value = allocate(element->value, -1, 1, sizeof(double));
		memcpy(element->value, value, sizeof(double));
	} else if (type == JSON_ARRAY) {
		jsonelement_t *array = value;
		element->size = array->size;
		element->value = allocate(element->value, -1, array->size, sizeof(jsonelement_t));

		for (size_t i = 0; i < array->size; ++i) {
			((jsonelement_t **) element->value)[i] = allocate(NULL, -1, 1, sizeof(jsonelement_t));
			jsonelement_t *array_element = ((jsonelement_t **) array->value)[i];
			jsonelement_t *sub_element = ((jsonelement_t **) element->value)[i];

			sub_element->parent = element;

			if (array_element->type == JSON_ARRAY || array_element->type == JSON_OBJECT) {
				set_value(sub_element, array_element, array_element->type);
			} else {
				set_value(sub_element, array_element->value, array_element->type);
			}
		}
	} else if (type == JSON_OBJECT) {
		jsonelement_t *object = value;
		element->size = object->size;
		element->value = allocate(element->value, -1, object->size, sizeof(jsonelement_t *));

		for (size_t i = 0; i < object->size; ++i) {
			((jsonelement_t **) element->value)[i] = allocate(NULL, -1, 1, sizeof(jsonelement_t));
			jsonelement_t *object_element = ((jsonelement_t **) object->value)[i];
			jsonelement_t *sub_element = ((jsonelement_t **) element->value)[i];

			sub_element->parent = element;
			sub_element->key = allocate(sub_element->key, -1, strlen(object_element->key) + 1, sizeof(char));
			strcpy(sub_element->key, object_element->key);

			if (object_element->type == JSON_ARRAY || object_element->type == JSON_OBJECT) {
				set_value(sub_element, object_element, object_element->type);
			} else {
				set_value(sub_element, object_element->value, object_element->type);
			}
		}
	}
}

void json_set_val(jsonelement_t *target, const char *key, void *value, const char type) {
	jsonelement_t *element = target;

	if (key == NULL) {
		++element->size;
		element->value = allocate(element->value, -1, element->size, sizeof(jsonelement_t *));
		((jsonelement_t **) element->value)[element->size - 1] = allocate(NULL, 0, 1, sizeof(jsonelement_t));
		((jsonelement_t **) element->value)[element->size - 1]->parent = element;

		element = ((jsonelement_t **) element->value)[element->size - 1];
		set_value(element, value, type);
	} else {
		Split splitter = split(key, ".");

		for (size_t i = 0; i < splitter.size; ++i) {
			const char *skey = splitter.data[i];
			const size_t skey_length = strlen(skey);

			char bwp[skey_length - 2 + 1];
			bwp[skey_length - 2] = 0;
			strncpy(bwp, skey + 1, skey_length - 2);
			const int iskey = atoi(bwp);

			if (skey[0] == '[' && skey[skey_length - 1] == ']' && (iskey > 0 || strcmp(bwp, "0") == 0)) {
				if (element->type != JSON_ARRAY) {
					if (element->type == JSON_OBJECT) {
						for (size_t n = 0; n < element->size; ++n) {
							json_free(((jsonelement_t **) element->value)[n], false);
						}
					}

					element->type = JSON_ARRAY;
					element->size = 0;
				}

				if (element->size < (iskey + 1)) {
					element->value = allocate(element->value, element->size, (iskey + 1), sizeof(jsonelement_t *));
					element->size = (iskey + 1);
					((jsonelement_t **) element->value)[iskey] = allocate(NULL, 0, 1, sizeof(jsonelement_t));
					((jsonelement_t **) element->value)[iskey]->parent = element;
				}

				if (i == (splitter.size - 1)) {
					element = ((jsonelement_t **) element->value)[iskey];
					set_value(element, value, type);
				} else {
					char new_key[strlen(key) - strlen(skey) + 1];
					new_key[0] = 0;

					element = ((jsonelement_t **) element->value)[iskey];
					join(splitter.data + 1, new_key, splitter.size - 1, ".");
					json_set_val(element, new_key, value, type);
				}
			} else {
				if (element->type != JSON_OBJECT) {
					if (element->type == JSON_ARRAY) {
						for (size_t n = 0; n < element->size; ++n) {
							json_free(((jsonelement_t **) element->value)[n], false);
						}
					}

					element->type = JSON_OBJECT;
					element->size = 0;
				}

				const size_t skey_length = strlen(skey);
				long at = -1;

				for (size_t n = 0; n < element->size; ++n) {
					if (strcmp(((jsonelement_t **) element->value)[n]->key, skey) == 0) {
						at = n;
						n = element->size;
					}
				}

				if (at == -1) {
					++element->size;
					element->value = allocate(element->value, element->size - 1, element->size, sizeof(jsonelement_t *));
					at = element->size - 1;
					((jsonelement_t **) element->value)[at] = allocate(((jsonelement_t **) element->value)[at], 0, 1, sizeof(jsonelement_t));
					((jsonelement_t **) element->value)[at]->parent = element;

					element = ((jsonelement_t **) element->value)[at];
					element->key = allocate(element->key, -1, skey_length + 1, sizeof(char));
					strcpy(element->key, skey);
				} else {
					element = ((jsonelement_t **) element->value)[at];
				}

				if (i == (splitter.size - 1)) {
					set_value(element, value, type);
				} else {
					char new_key[strlen(key) - strlen(skey) + 1];
					new_key[0] = 0;

					join(splitter.data + 1, new_key, splitter.size - 1, ".");
					json_set_val(element, new_key, value, type);
				}
			}
		}

		split_free(&splitter);
	}
}
