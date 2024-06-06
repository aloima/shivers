#include <utils.h>

int char_at(char *data, const char ch) {
	char *b = data;
	while (*data++ != ch && *data != '\0');
	return (*data == '\0') ? -1 : (data - b - 1);
}
