#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <json.h>
#include <utils.h>

jsonresult_t json_get_val(jsonelement_t *element, const char *search) {
	Split splitter = split(search, ".");
	jsonelement_t *value = element;
	jsonresult_t result = {0};
	result.exist = true;

	for (size_t ki = 0; ki < splitter.size; ++ki) {
		if (value->type == JSON_ARRAY) {
			int index = atoi(splitter.data[ki]);

			if (value->size > index) {
				value = ((jsonelement_t **) value->value)[index];
			} else {
				result.exist = false;
				break;
			}
		} else if (value->type == JSON_OBJECT) {
			size_t size = value->size;

			if (size == 0) {
				result.exist = false;
			} else {
				for (size_t i = 0; i < value->size; ++i) {
					jsonelement_t *data = ((jsonelement_t **) value->value)[i];

					if (strcmp(data->key, splitter.data[ki]) == 0) {
						value = data;
						i = value->size;
					} else if ((i + 1) == value->size) {
						result.exist = false;
						ki = splitter.size;
					}
				}
			}
		} else {
			result.exist = false;
			ki = splitter.size;
		}
	}

	split_free(&splitter);

	if (result.exist) {
		result.type = value->type;
		result.element = value;

		if (value->type == JSON_NUMBER) {
			result.value.number = ((double *) value->value)[0];
		} else if (value->type == JSON_STRING) {
			result.value.string = value->value;
		} else if (value->type == JSON_BOOLEAN) {
			result.value.boolean = ((bool *) value->value)[0];
		} else if (value->type == JSON_OBJECT) {
			result.value.object = value;
		} else if (value->type == JSON_ARRAY) {
			result.value.array = value;
		}
	} else {
		result = (jsonresult_t) {0};
	}

	return result;
}
