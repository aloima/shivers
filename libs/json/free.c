#include <stdlib.h>

#include <json.h>

void json_free(JSONElement *element) {
	size_t i;

	for (i = 0; i < element->size; ++i) {
		json_free(((JSONElement **) element->value)[i]);
	}

	element->parent = NULL;
	element->type = JSON_UNSPECIFIED;

	free(element->value);
	free(element->key);
	free(element);
}
