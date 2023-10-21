#include <stdlib.h>
#include <stdbool.h>

#include <utils.h>

void *allocate(void *value, const long old_count, const long new_count, const unsigned char size) {
	if (value == NULL) {
		return calloc(new_count, size); // remove it and fix uninitialization problem for strings
	} else {
		void *val = realloc(value, new_count * size);

		if (old_count != -1) {
			const long diff = (new_count - old_count);

			if (diff > 0) {
				memset(val + (old_count * size), 0, size * diff);
			}
		}

		return val;
	}
}
