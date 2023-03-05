#include <string.h>
#include <stdbool.h>

#include <utils.h>

char *join(char **value, size_t size, char *separator) {
	char *result = NULL;
	size_t result_length = 0;
	size_t separator_length = strlen(separator);

	for (size_t i = 0; i < size; ++i) {
		char *data = value[i];
		size_t length = strlen(data);
		bool has_separator = i != (size - 1);

		result_length += length + (has_separator ? separator_length : 0);
		result = allocate(result, sizeof(char), result_length + 1);
		strncat(result, data, length);

		if (has_separator) {
			strncat(result, separator, separator_length);
		}
	}

	return result;
}
