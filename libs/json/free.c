#include <stdlib.h>
#include <stdbool.h>

#include <json.h>

static void free_element(jsonelement_t *element) {
	if (element) {
		if (element->key) {
			free(element->key);
		}

		if (element->value) {
			free(element->value);
		}

		free(element);
	}
}

static void free_elements(jsonelement_t *parent) {
	if (parent) {
		if (parent->type == JSON_ARRAY || parent->type == JSON_OBJECT) {
			for (unsigned long i = 0; i < parent->size; ++i) {
				jsonelement_t *element = ((jsonelement_t **) parent->value)[i];

				if (element) {
					if (element->type == JSON_ARRAY || element->type == JSON_OBJECT) {
						free_elements(element);
					} else {
						free_element(element);
					}
				}
			}
		}

		free_element(parent);
	}
}

void json_free(jsonelement_t *element, const bool all) {
	jsonelement_t *top = element;

	if (all) {
		while (true) {
			if (top->parent) {
				top = top->parent;
			} else {
				break;
			}
		}
	}

	free_elements(top);
}
