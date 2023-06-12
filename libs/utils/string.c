#include <string.h>
#include <ctype.h>

#include <utils.h>

void strtolower(char *source, char *dest) {
	size_t length = strlen(dest);

	for (size_t i = 0; i < length; ++i) {
		char ch = dest[i];

		if (isupper(ch)) {
			source[i] = (ch - 32);
		} else {
			source[i] = ch;
		}
	}
}

void strtoupper(char *source, char *dest) {
	size_t length = strlen(dest);

	for (size_t i = 0; i < length; ++i) {
		char ch = dest[i];

		if (islower(ch)) {
			source[i] = (ch + 32);
		} else {
			source[i] = ch;
		}
	}
}