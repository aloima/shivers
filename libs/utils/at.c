#include <string.h>

int char_at(const char *data, const char ch) {
	size_t size = strlen(data);
	int result = -1;

	for (int i = 0; i < size; ++i) {
		if (data[i] == ch) {
			result = i;
			break;
		}
	}

	return result;
}
