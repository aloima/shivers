#include <stdlib.h>
#include <stdbool.h>

#include <json.h>

static void free_elements(jsonelement_t *parent) {
	for (size_t i = 0; i < parent->size; ++i) {
		jsonelement_t *element = ((jsonelement_t **) parent->value)[i];

		if (element) {
			if (element->type == JSON_ARRAY || element->type == JSON_OBJECT) {
				free_elements(element);
			} else {
				if (element->key) {
					free(element->key);
				}

				if (element->value) {
					free(element->value);
				}

				free(element);
			}
		}
	}

	if (parent->key) {
		free(parent->key);
	}

	if (parent->value) {
		free(parent->value);
	}

	free(parent);
}

void json_free(jsonelement_t *element) {
	jsonelement_t *top = element;

	while (true) {
		if (top->parent) {
			top = top->parent;
		} else {
			break;
		}
	}

	free_elements(top);
}
