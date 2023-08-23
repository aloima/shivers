#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#include <json.h>
#include <utils.h>

JSONResult json_get_val(JSONElement *element, const char *search) {
	Split splitter = split((char *) search, ".");
	JSONElement *value = element;
	JSONResult result = {0};
	result.exist = true;

	for (size_t ki = 0; ki < splitter.size; ++ki) {
		if (value->type == JSON_ARRAY) {
			int index = atoi(splitter.data[ki]);

			if (value->size > index) {
				value = ((JSONElement **) value->value)[index];
			} else {
				result.exist = false;
				break;
			}
		} else if (value->type == JSON_OBJECT) {
			for (size_t i = 0; i < value->size; ++i) {
				JSONElement *data = ((JSONElement **) value->value)[i];

				if (strcmp(data->key, splitter.data[ki]) == 0) {
					value = data;
					i = value->size;
				} else if ((i + 1) == value->size) {
					result.exist = false;
					ki = splitter.size;
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

		if (value->type == JSON_NUMBER) {
			if (value->size == 2) {
				long fractional = ((long *) value->value)[1];
				int fractional_digit_count = (floor(log10(fractional)) + 1);

				result.value.number = (((long *) value->value)[0] + (fractional * pow(10, -fractional_digit_count)));
			} else {
				result.value.number = ((long *) value->value)[0];
			}
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
		result = (JSONResult) {0};
	}

	return result;
}
