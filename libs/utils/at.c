#include <string.h>

#include <utils.h>

int char_at(const char *data, const char ch, int length) {
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
