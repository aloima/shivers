#include <string.h>
#include <ctype.h>

#include <utils.h>

void strtolower(char *source, const char *dest) {
	const unsigned long length = strlen(dest);

	for (unsigned long i = 0; i < length; ++i) {
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
	const unsigned long length = strlen(dest);

	for (unsigned long i = 0; i < length; ++i) {
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
	unsigned long i = 0;

	while (isblank(src[i])) {
		++i;
	}

	return (src + i);
}
