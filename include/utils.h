#ifndef UTIL_H_
	#define UTIL_H_

	typedef struct {
		char **data;
		size_t size;
	} Split;

	void split_free(Split *value);

	Split split(char *text, char *separator);
	char *join(char **value, size_t size, char *separator);
	void *allocate(void *value, size_t count, size_t size);
#endif
