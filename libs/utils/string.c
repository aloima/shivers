#include <string.h>
#include <ctype.h>

#include <utils.h>

void strtolower(char *source, const char *dest) {
	const size_t length = strlen(dest);

	for (size_t i = 0; i < length; ++i) {
		const char ch = dest[i];

		if (isupper(ch)) {
			source[i] = tolower(ch);
		} else {
			source[i] = ch;
		}
	}

	source[length] = '\0';
}

void strtoupper(char *source, const char *dest) {
	const size_t length = strlen(dest);

	for (size_t i = 0; i < length; ++i) {
		const char ch = dest[i];

		if (islower(ch)) {
			source[i] = toupper(ch);
		} else {
			source[i] = ch;
		}
	}

	source[length] = '\0';
}

const char *ltrim(const char *src) {
	size_t i = 0;

	while (isblank(src[i])) {
		++i;
	}

	return (src + i);
}
