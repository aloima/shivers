#include <string.h>

long char_at(const char *data, const char ch) {
	const unsigned long size = strlen(data);
	long result = -1;

	for (unsigned long i = 0; i < size; ++i) {
		if (data[i] == ch) {
			result = i;
			break;
		}
	}

	return result;
}
