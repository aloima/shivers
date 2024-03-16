#include <string.h>
#include <stdbool.h>

#include <utils.h>

unsigned long calculate_join(const struct Join *value, unsigned short size, const char *separator) {
	unsigned long source_length = (strlen(separator) * (size - 1));

	for (unsigned short i = 0; size != 0; ++i) {
		const unsigned long string_length = value[i].length;

		if (string_length != 0) {
			source_length += string_length;
			--size;
		}
	}

	return source_length;
}

unsigned long join(const struct Join *value, char *source, unsigned short size, const char *separator) {
	const unsigned long separator_length = strlen(separator);
	unsigned long source_length = 0;

	for (unsigned short i = 0; size != 0; ++i) {
		const struct Join join_element = value[i];
		const bool has_separator = (size != 1);

		if (join_element.length != 0) {
			memcpy(source + source_length, join_element.data, join_element.length);

			if (has_separator) {
				memcpy(source + source_length + join_element.length, separator, separator_length);
			}

			--size;
			source_length += (join_element.length + (has_separator ? separator_length : 0));
		}
	}

	source[source_length] = 0;

	return source_length;
}
