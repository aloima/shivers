#include <string.h>

#include <utils.h>

size_t calculate_join(char **value, size_t size, char *separator) {
	size_t source_length = 0, i, separator_length = strlen(separator);

	for (i = 0; i < size; ++i) {
		source_length += strlen(value[i]);

		if ((i + 1) != size) {
			source_length += separator_length;
		}
	}

	return source_length;
}

size_t join(char **value, char *source, size_t size, char *separator) {
	size_t source_length = 0;
	size_t separator_length = strlen(separator);

	size_t i;

	for (i = 0; i < size; ++i) {
		char *data = value[i];
		size_t length = strlen(data);
		bool has_separator = i != (size - 1);

		source_length += length + (has_separator ? separator_length : 0);
		strncat(source, data, length);

		if (has_separator) {
			strncat(source, separator, separator_length);
		}
	}

	return source_length;
}
