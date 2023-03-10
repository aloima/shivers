#include <stdlib.h>

#include <json.h>

void json_free(JSONElement *element) {
	size_t i;

	for (i = 0; i < element->size; ++i) {
		json_free(((JSONElement **) element->value)[i]);
	}

	free(element->value);
	free(element->key);
	free(element);
}
