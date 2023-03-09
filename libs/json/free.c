#include <stdlib.h>

#include <json.h>

void json_free(JSONElement *element) {
	element->parent = NULL;
	element->type = JSON_UNSPECIFIED;

	for (size_t i = 0; i < element->size; ++i) {
		json_free(((JSONElement **) element->value)[i]);
	}

	free(element->value);
	free(element->key);
	free(element);
}
