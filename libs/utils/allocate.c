#include <stdlib.h>

#include <utils.h>

void *allocate(void *value, size_t count, size_t size) {
	if (value == NULL) {
		value = calloc(count, size);
	} else {
		value = realloc(value, count * size);
	}

	return value;
}
