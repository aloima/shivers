#include <stdlib.h>

#include <utils.h>

void *allocate(void *value, const size_t old_count, const size_t new_count, const size_t size) {
	if (value == NULL) {
		return calloc(new_count, size);
	} else {
		void *val = realloc(value, new_count * size);
		long diff = (new_count - old_count);

		if (diff > 0) {
			memset(val + (old_count * size), 0, size * diff);
		}

		return val;
	}
}
