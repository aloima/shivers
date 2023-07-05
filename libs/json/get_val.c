#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#include <json.h>
#include <utils.h>

JSONValue json_get_val(JSONElement *element, const char *search) {
	Split splitter = split((char *) search, ".");
	JSONElement *value = element;
	JSONValue result;

	memset(&result, 0, sizeof(JSONValue));

	for (size_t ki = 0; ki < splitter.size; ++ki) {
		for (size_t i = 0; i < value->size; ++i) {
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
		if (value->size == 2) {
			int fractional_digit_count = floorl(log10(floorf(((long *) value->value)[1]))) + 1;
			result.number = (((long *) value->value)[0] + (((long *) value->value)[1] * pow(10, -fractional_digit_count)));
		} else {
			result.number = ((long *) value->value)[0];
		}
	} else if (value->type == JSON_STRING) {
		result.string = value->value;
	} else if (value->type == JSON_BOOLEAN) {
		result.boolean = ((bool *) value->value)[0];
	} else if (value->type == JSON_OBJECT) {
		result.object = value;
	} else if (value->type == JSON_ARRAY) {
		result.array = value;
	}

	return result;
}
