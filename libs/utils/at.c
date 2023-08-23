#include <string.h>

const size_t char_at(const char *data, const char ch) {
	const size_t size = strlen(data);
	size_t result = -1;

	for (size_t i = 0; i < size; ++i) {
		if (data[i] == ch) {
			result = i;
			break;
		}
	}

	return result;
}
