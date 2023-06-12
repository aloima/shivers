#include <string.h>
#include <stdbool.h>

#include <utils.h>

size_t calculate_join(char **value, size_t size, char *separator) {
	size_t source_length = (strlen(separator) * (size - 1));

	for (size_t i = 0; i < size; ++i) {
		source_length += strlen(value[i]);
	}

	return source_length;
}

size_t join(char **value, char *source, size_t size, char *separator) {
	size_t separator_length = strlen(separator);
	size_t source_length = (separator_length * (size - 1));

	for (size_t i = 0; i < size; ++i) {
		char *data = value[i];
		size_t length = strlen(data);
		bool has_separator = (i != (size - 1));

		strncat(source, data, length);

		if (has_separator) {
			strncat(source, separator, separator_length);
		}
	}

	return source_length;
}
