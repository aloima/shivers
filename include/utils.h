#include <string.h>

#ifndef UTIL_H_
	#define UTIL_H_

	typedef struct {
		char **data;
		size_t size;
	} Split;

	void split_free(Split *value);

	Split split(char *text, char *separator);
	size_t join(char **value, char *source, size_t size, char *separator);
	size_t calculate_join(char **value, size_t size, char *separator);
	void *allocate(void *value, size_t count, size_t size);

	void strtolower(char *source, char *dest);
	void strtoupper(char *source, char *dest);
#endif
