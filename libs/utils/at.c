#include <utils.h>

int char_at(char *data, const char ch) {
	char *b = data;
	while (*data != ch && *data != '\0') data++;

	const unsigned int diff = (data - b);
	return b[diff] == '\0' ? -1 : diff;
}
