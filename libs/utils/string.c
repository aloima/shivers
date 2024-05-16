#include <string.h>
#include <ctype.h>
#include <stdbool.h>

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

bool strsame(const char *str1, const char *str2) {
	while ((*str1) != '\0') {
		if ((*str1++) != (*str2++)) {
			return false;
		}
	}

	return ((*str2) == '\0');
}

void strreplace(char **source, char *target, char *replacement) {
	const unsigned int replacement_length = strlen(replacement);
	const unsigned int target_length = strlen(target);
	const int diff = (replacement_length - target_length);
	unsigned int source_length = strlen(*source);

	char temp[source_length + 1];

	if (diff > 0) {
		char *pointer = NULL;

		while (true) {
			*source = allocate(*source, source_length + 1, source_length + diff + 1, sizeof(char));
			source_length += diff;
			pointer = strstr(*source, target);

			if (pointer != NULL) {
				strcpy(temp, pointer + target_length);
				strcpy(pointer + replacement_length, temp);
				strncpy(pointer, replacement, replacement_length);

				pointer = strstr(*source, target);
			} else {
				*source = allocate(*source, source_length + 1, source_length - diff + 1, sizeof(char));
				break;
			}
		}
	} else if (diff < 0) {
		char *pointer = strstr(*source, target);

		while (pointer != NULL) {
			strcpy(temp, pointer + target_length);
			strcpy(pointer + replacement_length, temp);
			strncpy(pointer, replacement, replacement_length);

			*source = allocate(*source, source_length + 1, source_length + diff + 1, sizeof(char));
			source_length += diff;
			pointer = strstr(*source, target);
		}
	} else {
		char *pointer = strstr(*source, target);

		while (pointer != NULL) {
			strncpy(pointer, replacement, replacement_length);
			pointer = strstr(*source, target);
		}
	}
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

char *ltrim(const char *src) {
	while (isblank(*src)) ++src;
	return (char *) src;
}
