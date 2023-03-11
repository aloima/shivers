#include <stdlib.h>
#include <string.h>

#include <utils.h>

void split_free(Split *value) {
	size_t i;

	for (i = 0; i < value->size; ++i) {
		free(value->data[i]);
	}

	free(value->data);
}

Split split(char *text, char *separator) {
	size_t length = strlen(text);
	size_t separator_length = strlen(separator);
	size_t data_length = 0;
	size_t i;

	Split result;
	result.data = allocate(NULL, 1, sizeof(char *));
	result.size = 1;

	for (i = 0; i < length; ++i) {
		char **data = &result.data[result.size - 1];
		char ch = text[i];

		if (ch != separator[0]) {
			++data_length;
			*data = allocate(data_length == 1 ? NULL : *data, data_length + 1, sizeof(char));

			strncat(*data, &ch, 1);
		} else {
			bool completed = true;

			size_t _i;

			for (_i = 1; _i < separator_length; ++_i) {
				if (separator[_i] != text[i + _i]) {
					completed = false;
					_i = separator_length;
				}
			}

			if (completed) {
				i += separator_length - 1;
				data_length = 0;
				++result.size;
				result.data = allocate(result.data, result.size, sizeof(char *));
			} else {
				++data_length;
				*data = allocate(*data, data_length + 1, sizeof(char));

				strncat(*data, &ch, 1);
			}
		}
	}

	return result;
}
