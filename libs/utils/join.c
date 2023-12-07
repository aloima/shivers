#include <string.h>
#include <stdbool.h>

#include <utils.h>

size_t calculate_join(const struct Join *value, unsigned short size, const char *separator) {
	size_t source_length = (strlen(separator) * (size - 1));

	for (unsigned short i = 0; size != 0; ++i) {
		const size_t string_length = value[i].length;

		if (string_length != 0) {
			source_length += string_length;
			--size;
		}
	}

	return source_length;
}

size_t join(const struct Join *value, char *source, unsigned short size, const char *separator) {
	const size_t separator_length = strlen(separator);
	size_t source_length = 0;

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

void create_join_elements_nz(struct Join *joins, const char **list, const size_t list_size) {
	for (size_t i = 0; i < list_size; ++i) {
		joins[i].data = (char *) list[i];
		joins[i].length = ((list[i] == NULL) ? 0 : strlen(list[i]));
	}
}
