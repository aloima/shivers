#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <json.h>
#include <utils.h>

char *json_stringify(JSONElement *element) {
	char *result = NULL;

	if (element->type == JSON_STRING) {
		size_t length = strlen(element->value);
		result = allocate(result, length + 3, sizeof(char));
		strncat(result, "\"", 1);
		strncat(result, element->value, length);
		strncat(result, "\"", 1);
	} else if (element->type == JSON_NUMBER) {
		long double number = (long double) ((long *) element->value)[0];
		double digit_count = 1.00;

		while ((number / (long double) pow(10, digit_count)) >= 1) {
			++digit_count;
		}

		result = allocate(result, (size_t) (digit_count + 1), sizeof(char));

		while ((size_t) digit_count != 0) {
			double digit = floor((double) (number / (long double) pow(10, (digit_count - 1))));
			sprintf(result, "%s%0.0f", result, digit);
			number -= (long double) (digit * pow(10, (digit_count - 1)));
			--digit_count;
		}
	} else if (element->type == JSON_BOOLEAN) {
		bool value = ((bool *) element->value)[0];

		if (value) {
			result = allocate(result, 5, sizeof(char));
			strncat(result, "true", 4);
		} else {
			result = allocate(result, 6, sizeof(char));
			strncat(result, "false", 5);
		}
	} else if (element->type == JSON_NULL) {
		result = allocate(result, 5, sizeof(char));
		strncat(result, "null", 4);
	} else if (element->type == JSON_OBJECT) {
		size_t result_length = 2;
		size_t i;

		result = allocate(result, result_length + 1, sizeof(char));
		strncat(result, "{", 1);

		for (i = 0; i < element->size; ++i) {
			JSONElement *data = ((JSONElement **) element->value)[i];
			char *value = json_stringify(data);
			size_t key_length = strlen(data->key);
			size_t value_length = strlen(value);
			bool has_comma = (element->size != (i + 1));

			result_length += key_length + 3 + value_length + has_comma;
			result = allocate(result, result_length + 1, sizeof(char));
			strncat(result, "\"", 1);
			strncat(result, data->key, key_length);
			strncat(result, "\":", 2);
			strncat(result, value, value_length);

			if (has_comma) {
				strncat(result, ",", 1);
			}

			free(value);
		}

		strncat(result, "}", 1);
	} else if (element->type == JSON_ARRAY) {
		size_t result_length = 2;
		size_t i;

		result = allocate(result, result_length + 1, sizeof(char));
		strncat(result, "[", 1);

		for (i = 0; i < element->size; ++i) {
			JSONElement *data = ((JSONElement **) element->value)[i];
			char *value = json_stringify(data);
			size_t value_length = strlen(value);
			bool has_comma = (element->size != (i + 1));

			result_length += value_length + has_comma;
			result = allocate(result, result_length + 1, sizeof(char));
			strncat(result, value, value_length);

			if (has_comma) {
				strncat(result, ",", 1);
			}

			free(value);
		}

		strncat(result, "]", 1);
	}

	return result;
}
