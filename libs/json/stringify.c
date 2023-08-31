#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>

#include <json.h>
#include <utils.h>

char *json_stringify(jsonelement_t *element) {
	char *result = NULL;

	if (element->type == JSON_STRING) {
		size_t length = strlen(element->value);
		result = allocate(NULL, 0, length + 3, sizeof(char));
		result[0] = '"';
		strncat(result, element->value, length);
		result[length + 1] = '"';
	} else if (element->type == JSON_NUMBER) {
		long number = ((long *) element->value)[0];
		size_t digit_count = (floor(log10(number)) + 1);
		size_t i = digit_count;

		result = allocate(NULL, 0, (digit_count + 1), sizeof(char));

		while (i != 0) {
			int8_t digit = floor(number / pow(10, (i - 1)));
			char ch = (digit + 48);
			strncat(result, &ch, 1);
			number -= (digit * pow(10, (i - 1)));
			--i;
		}

		if (element->size == 2) {
			long fractional_number = ((long *) element->value)[1];
			size_t fractional_digit_count = floor(log10(fractional_number)) + 1;
			result = allocate(result, digit_count + 1, digit_count + fractional_digit_count + 2, sizeof(char));
			result[digit_count] = '.';

			while (fractional_digit_count != 0) {
				int8_t digit = floor(fractional_number / pow(10, (fractional_digit_count - 1)));
				char ch = (digit + 48);
				strncat(result, &ch, 1);
				fractional_number -= (digit * pow(10, (fractional_digit_count - 1)));
				--fractional_digit_count;
			}
		}
	} else if (element->type == JSON_BOOLEAN) {
		bool value = ((bool *) element->value)[0];

		if (value) {
			result = allocate(NULL, 0, 5, sizeof(char));
			strncat(result, "true", 4);
		} else {
			result = allocate(NULL, 0, 6, sizeof(char));
			strncat(result, "false", 5);
		}
	} else if (element->type == JSON_NULL) {
		result = allocate(NULL, 0, 5, sizeof(char));
		strncat(result, "null", 4);
	} else if (element->type == JSON_OBJECT) {
		size_t result_length = 2;
		size_t i;

		result = allocate(NULL, 0, result_length + 1, sizeof(char));
		strncat(result, "{", 1);

		for (i = 0; i < element->size; ++i) {
			jsonelement_t *data = ((jsonelement_t **) element->value)[i];
			char *value = json_stringify(data);
			size_t key_length = strlen(data->key);
			size_t value_length = strlen(value);
			bool has_comma = (element->size != (i + 1));

			result_length += key_length + 3 + value_length + has_comma;
			result = allocate(result, result_length - key_length - 3 - value_length - has_comma, result_length + 1, sizeof(char));
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

		result = allocate(NULL, 0, result_length + 1, sizeof(char));
		strncat(result, "[", 1);

		for (i = 0; i < element->size; ++i) {
			jsonelement_t *data = ((jsonelement_t **) element->value)[i];
			char *value = json_stringify(data);
			size_t value_length = strlen(value);
			bool has_comma = (element->size != (i + 1));

			result_length += value_length + has_comma;
			result = allocate(result, result_length - value_length - has_comma + 1, result_length + 1, sizeof(char));
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
