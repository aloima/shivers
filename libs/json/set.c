#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include <utils.h>
#include <json.h>

static void set_value(jsonelement_t *element, void *value, const unsigned char type) {
	if (element->type == JSON_ARRAY || element->type == JSON_OBJECT) {
		for (unsigned int i = 0; i < element->size; ++i) {
			json_free(((jsonelement_t **) element->value)[i], false);
		}
	}

	element->type = type;

	if (type == JSON_STRING) {
		const unsigned int value_length = strlen(value);

		element->size = value_length;
		element->value = allocate(element->value, -1, value_length + 1, sizeof(char));
		memcpy(element->value, value, value_length + 1);
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

		for (unsigned int i = 0; i < array->size; ++i) {
			((jsonelement_t **) element->value)[i] = allocate(NULL, 0, 1, sizeof(jsonelement_t));
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

		for (unsigned int i = 0; i < object->size; ++i) {
			((jsonelement_t **) element->value)[i] = allocate(NULL, 0, 1, sizeof(jsonelement_t));
			jsonelement_t *object_element = ((jsonelement_t **) object->value)[i];
			jsonelement_t *sub_element = ((jsonelement_t **) element->value)[i];
			const unsigned int key_size = (strlen(object_element->key) + 1);

			sub_element->parent = element;
			sub_element->key = allocate(sub_element->key, -1, key_size, sizeof(char));
			memcpy(sub_element->key, object_element->key, key_size);

			if (object_element->type == JSON_ARRAY || object_element->type == JSON_OBJECT) {
				set_value(sub_element, object_element, object_element->type);
			} else {
				set_value(sub_element, object_element->value, object_element->type);
			}
		}
	}
}

void json_set_val(jsonelement_t *target, const char *key, void *value, const enum JSONType type) {
	if (key == NULL) {
		++target->size;
		target->value = allocate(target->value, -1, target->size, sizeof(jsonelement_t *));
		((jsonelement_t **) target->value)[target->size - 1] = allocate(NULL, 0, 1, sizeof(jsonelement_t));
		((jsonelement_t **) target->value)[target->size - 1]->parent = target;

		target = ((jsonelement_t **) target->value)[target->size - 1];
		set_value(target, value, type);
	} else {
		const unsigned int key_length = strlen(key);
		struct Split splitter = split(key, key_length, ".");

		const unsigned int loop_index_bound = (splitter.size - 1);

		for (unsigned long i = 0; i < splitter.size; ++i) {
			const char *skey = splitter.data[i].data;
			const unsigned int skey_length = splitter.data[i].length;

			const unsigned int bwp_length = (skey_length - 2);
			char bwp[bwp_length + 1];
			bwp[bwp_length] = 0;
			strncpy(bwp, skey + 1, bwp_length);
			const int iskey = atoi_s(bwp, bwp_length);

			if (skey[0] == '[' && skey[skey_length - 1] == ']' && (iskey > 0 || strsame(bwp, "0"))) {
				if (target->type != JSON_ARRAY) {
					if (target->type == JSON_OBJECT) {
						for (unsigned int n = 0; n < target->size; ++n) {
							json_free(((jsonelement_t **) target->value)[n], false);
						}
					}

					target->type = JSON_ARRAY;
					target->size = 0;
				}

				if (target->size < (iskey + 1)) {
					target->value = allocate(target->value, target->size, (iskey + 1), sizeof(jsonelement_t *));
					target->size = (iskey + 1);
					((jsonelement_t **) target->value)[iskey] = allocate(NULL, 0, 1, sizeof(jsonelement_t));
					((jsonelement_t **) target->value)[iskey]->parent = target;
				}

				if (i == loop_index_bound) {
					target = ((jsonelement_t **) target->value)[iskey];
					set_value(target, value, type);
				} else {
					char new_key[key_length - skey_length + 1];

					target = ((jsonelement_t **) target->value)[iskey];
					join((struct Join *) (splitter.data + 1), new_key, splitter.size - 1, ".");

					json_set_val(target, new_key, value, type);
				}
			} else {
				if (target->type != JSON_OBJECT) {
					if (target->type == JSON_ARRAY) {
						for (unsigned int n = 0; n < target->size; ++n) {
							json_free(((jsonelement_t **) target->value)[n], false);
						}
					}

					target->type = JSON_OBJECT;
					target->size = 0;
				}

				long at = -1;

				for (unsigned int n = 0; n < target->size; ++n) {
					if (strsame(((jsonelement_t **) target->value)[n]->key, skey)) {
						at = n;
						break;
					}
				}

				if (at == -1) {
					++target->size;
					target->value = allocate(target->value, target->size - 1, target->size, sizeof(jsonelement_t *));
					at = target->size - 1;
					((jsonelement_t **) target->value)[at] = allocate(((jsonelement_t **) target->value)[at], 0, 1, sizeof(jsonelement_t));
					((jsonelement_t **) target->value)[at]->parent = target;

					target = ((jsonelement_t **) target->value)[at];
					target->key = allocate(target->key, -1, skey_length + 1, sizeof(char));
					memcpy(target->key, skey, skey_length + 1);
				} else {
					target = ((jsonelement_t **) target->value)[at];
				}

				if (i == loop_index_bound) {
					set_value(target, value, type);
				} else {
					char new_key[key_length - skey_length + 1];

					join((struct Join *) (splitter.data + 1), new_key, splitter.size - 1, ".");
					json_set_val(target, new_key, value, type);
					i = splitter.size;
				}
			}
		}

		split_free(splitter);
	}
}
