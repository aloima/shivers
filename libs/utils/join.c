#include <string.h>
#include <stdbool.h>

#include <utils.h>

size_t calculate_join(char **value, unsigned short size, const char *separator) {
	size_t source_length = (strlen(separator) * (size - 1));

	for (unsigned short i = 0; size != 0; ++i) {
		const size_t string_length = strlen(value[i]);

		if (string_length != 0) {
			source_length += string_length;
			--size;
		}
	}

	return source_length;
}

size_t join(char **value, char *source, unsigned short size, const char *separator) {
	const size_t separator_length = strlen(separator);
	const size_t source_length = (separator_length * (size - 1));

	for (unsigned short i = 0; size != 0; ++i) {
		const char *data = value[i];
		const size_t length = strlen(data);
		const bool has_separator = (size != 1);

		if (length != 0) {
			strncat(source, data, length);

			if (has_separator) {
				strncat(source, separator, separator_length);
			}

			--size;
		}
	}

	return source_length;
}
