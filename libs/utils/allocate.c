#include <stdlib.h>

#include <utils.h>

void *allocate(void *value, size_t count, size_t size) {
	if (value == NULL) {
		return calloc(count, size);
	} else {
		return realloc(value, count * size);
	}
}
