#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#include <json.h>
#include <utils.h>

char *json_stringify(const jsonelement_t *element, const unsigned char fractional_limit) {
	char *result = NULL;

	if (!element || element->type == JSON_UNSPECIFIED) {
		result = allocate(NULL, -1, 8, sizeof(char));
		strcpy(result, "<empty>");
	} else if (element->type == JSON_STRING) {
		const size_t length = strlen(element->value);
		result = allocate(NULL, -1, length + 3, sizeof(char));
		result[0] = '"';
		strncat(result, element->value, length);
		result[length + 1] = '"';
		result[length + 2] = '\0';
	} else if (element->type == JSON_NUMBER) {
		const double number = ((double *) element->value)[0];
		double integer;
		double fractional = (modf(number, &integer) * pow(10, fractional_limit));

		const size_t digit_count = (floor(log10(integer)) + 1);
		char formatter[12];

		if (fractional != 0.0) {
			result = allocate(NULL, -1, digit_count + fractional_limit + 2, sizeof(char));
			sprintf(formatter, "%%.0f.%%%d.0f", fractional_limit);
			sprintf(result, formatter, integer, fractional);
		} else {
			result = allocate(NULL, -1, (digit_count + 1), sizeof(char));
			strcpy(formatter, "%.0f");
			sprintf(result, formatter, integer);
		}
	} else if (element->type == JSON_BOOLEAN) {
		const bool value = ((bool *) element->value)[0];

		if (value) {
			result = allocate(NULL, -1, 5, sizeof(char));
			strncat(result, "true", 4);
		} else {
			result = allocate(NULL, -1, 6, sizeof(char));
			strncat(result, "false", 5);
		}
	} else if (element->type == JSON_NULL) {
		result = allocate(NULL, -1, 5, sizeof(char));
		strncat(result, "null", 4);
	} else if (element->type == JSON_OBJECT) {
		size_t result_length = 2;
		size_t i;

		result = allocate(NULL, -1, result_length + 1, sizeof(char));
		strncat(result, "{", 1);

		for (i = 0; i < element->size; ++i) {
			const jsonelement_t *data = ((jsonelement_t **) element->value)[i];
			char *value = json_stringify(data, fractional_limit);
			const size_t key_length = strlen(data->key);
			const size_t value_length = strlen(value);
			const bool has_comma = (element->size != (i + 1));

			result_length += key_length + 3 + value_length + has_comma;
			result = allocate(result, -1, result_length + 1, sizeof(char));
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

		result = allocate(NULL, -1, result_length + 1, sizeof(char));
		strncat(result, "[", 1);

		for (i = 0; i < element->size; ++i) {
			const jsonelement_t *data = ((jsonelement_t **) element->value)[i];
			char *value = json_stringify(data, fractional_limit);
			const size_t value_length = strlen(value);
			const bool has_comma = (element->size != (i + 1));

			result_length += value_length + has_comma;
			result = allocate(result, -1, result_length + 1, sizeof(char));
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
