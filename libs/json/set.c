#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include <utils.h>
#include <json.h>

static void set_value(jsonelement_t *element, void *value, const unsigned char type) {
	if (element->type == JSON_ARRAY || element->type == JSON_OBJECT) {
		for (unsigned long i = 0; i < element->size; ++i) {
			json_free(((jsonelement_t **) element->value)[i], false);
		}
	}

	element->type = type;

	if (type == JSON_STRING) {
		const unsigned long value_size = (strlen(value) + 1);

		element->value = allocate(element->value, -1, value_size, sizeof(char));
		memcpy(element->value, value, value_size);
	} else if (type == JSON_BOOLEAN) {
		element->value = allocate(element->value, -1, 1, sizeof(char));
		((bool *) element->value)[0] = *((bool *) value);
	} else if (type == JSON_NUMBER) {
		element->value = allocate(element->value, -1, 1, sizeof(double));
		((double *) element->value)[0] = *((double *) value);
	} else if (type == JSON_ARRAY) {
		jsonelement_t *array = value;
		element->size = array->size;
		element->value = allocate(element->value, -1, array->size, sizeof(jsonelement_t));

		for (unsigned long i = 0; i < array->size; ++i) {
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

		for (unsigned long i = 0; i < object->size; ++i) {
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
		const unsigned long key_length = strlen(key);
		struct Split splitter = split(key, key_length, ".");

		const unsigned long loop_index_bound = (splitter.size - 1);

		for (unsigned long i = 0; i < splitter.size; ++i) {
			const char *skey = splitter.data[i].data;
			const unsigned long skey_length = splitter.data[i].length;

			const unsigned long bwp_length = (skey_length - 2);
			char bwp[bwp_length + 1];
			bwp[bwp_length] = 0;
			strncpy(bwp, skey + 1, bwp_length);
			const int iskey = atoi_s(bwp, bwp_length);

			if (skey[0] == '[' && skey[skey_length - 1] == ']' && (iskey > 0 || strsame(bwp, "0"))) {
				if (element->type != JSON_ARRAY) {
					if (element->type == JSON_OBJECT) {
						for (unsigned long n = 0; n < element->size; ++n) {
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

				if (i == loop_index_bound) {
					element = ((jsonelement_t **) element->value)[iskey];
					set_value(element, value, type);
				} else {
					char new_key[key_length - skey_length + 1];

					element = ((jsonelement_t **) element->value)[iskey];
					join((struct Join *) (splitter.data + 1), new_key, splitter.size - 1, ".");

					json_set_val(element, new_key, value, type);
				}
			} else {
				if (element->type != JSON_OBJECT) {
					if (element->type == JSON_ARRAY) {
						for (unsigned long n = 0; n < element->size; ++n) {
							json_free(((jsonelement_t **) element->value)[n], false);
						}
					}

					element->type = JSON_OBJECT;
					element->size = 0;
				}

				long at = -1;

				for (unsigned long n = 0; n < element->size; ++n) {
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

				if (i == loop_index_bound) {
					set_value(element, value, type);
				} else {
					char new_key[key_length - skey_length + 1];

					join((struct Join *) (splitter.data + 1), new_key, splitter.size - 1, ".");
					json_set_val(element, new_key, value, type);
					i = splitter.size;
				}
			}
		}

		split_free(splitter);
	}
}
