#include <stdlib.h>

#include <json.h>

void json_free(JSONElement *element) {
	if (element->type != JSON_NUMBER) {
		for (size_t i = 0; i < element->size; ++i) {
			json_free(((JSONElement **) element->value)[i]);
		}
	}

	if (element->key) {
		free(element->key);
	}

	if (element->value) {
		free(element->value);
	}

	if (element) {
		free(element);
	}
}
