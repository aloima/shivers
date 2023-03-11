#include <stdlib.h>
#include <string.h>

#include <json.h>
#include <utils.h>

JSONValue json_get_val(JSONElement *element, char *search) {
	Split splitter = split(search, ".");
	JSONElement *value = element;
	JSONValue result;
	size_t ki;

	memset(&result, 0, sizeof(JSONValue));

	for (ki = 0; ki < splitter.size; ++ki) {
		size_t i;

		for (i = 0; i < value->size; ++i) {
			if (value->type == JSON_OBJECT) {
				JSONElement *data = ((JSONElement **) value->value)[i];

				if (strcmp(data->key, splitter.data[ki]) == 0) {
					value = data;
					i = value->size;
				}
			} else if (value->type == JSON_ARRAY) {
				value = ((JSONElement **) value->value)[atoi(splitter.data[ki])];
				i = value->size;
			}
		}
	}

	split_free(&splitter);

	result.type = value->type;

	if (value->type == JSON_NUMBER) {
		result.number = ((long *) value->value)[0];
	} else if (value->type == JSON_STRING) {
		result.string = value->value;
	} else if (value->type == JSON_BOOLEAN) {
		result.boolean = ((bool *) value->value)[0];
	}

	return result;
}
