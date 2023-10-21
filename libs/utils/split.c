#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <utils.h>

void split_free(Split *value) {
	for (size_t i = 0; i < value->size; ++i) {
		free(value->data[i]);
	}

	free(value->data);
}

Split split(const char *text, const char *separator) {
	const size_t length = strlen(text);
	const size_t separator_length = strlen(separator);
	size_t data_length = 0;

	Split result = {
		.data = allocate(NULL, -1, 1, sizeof(char *)),
		.size = 1
	};

	for (size_t i = 0; i < length; ++i) {
		char **data = &result.data[result.size - 1];
		const char ch = text[i];

		if (ch != separator[0]) {
			++data_length;
			*data = allocate(data_length == 1 ? NULL : *data, -1, data_length + 1, sizeof(char));

			strncat(*data, &ch, 1);
		} else {
			bool completed = true;

			for (size_t _i = 1; _i < separator_length; ++_i) {
				if (separator[_i] != text[i + _i]) {
					completed = false;
					_i = separator_length;
				}
			}

			if (completed) {
				if (data_length == 0) {
					result.data[result.size - 1] = allocate(NULL, 0, 1, sizeof(char));
				}

				i += separator_length - 1;
				data_length = 0;
				++result.size;
				result.data = allocate(result.data, result.size - 1, result.size, sizeof(char *));

				if (length == (i + 1)) {
					result.data[result.size - 1] = allocate(NULL, 0, 1, sizeof(char));
				}
			} else {
				++data_length;
				*data = allocate(*data, -1, data_length + 1, sizeof(char));

				strncat(*data, &ch, 1);
			}
		}
	}

	return result;
}
