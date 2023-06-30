#include <string.h>

#ifndef UTIL_H_
	#define UTIL_H_

	typedef struct {
		char **data;
		size_t size;
	} Split;

	Split split(char *text, char *separator);
	void split_free(Split *value);

	void *allocate(void *value, size_t count, size_t size);

	size_t join(char **value, char *source, size_t size, char *separator);
	size_t calculate_join(char **value, size_t size, char *separator);

	void strtolower(char *source, char *dest);
	void strtoupper(char *source, char *dest);

	char *base64_encode(const char *data);
#endif
