#include <string.h>

long char_at(const char *data, const char ch, unsigned long length) {
	if (length == 0) {
		length = strlen(data);
	}

	for (unsigned long i = 0; i < length; ++i) {
		if (data[i] == ch) {
			return i;
		}
	}

	return -1;
}
