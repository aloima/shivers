#include <stdlib.h>
#include <string.h>

#include <utils.h>

void *allocate(void *value, const long old_count, const unsigned long new_count, const unsigned char size) {
	if (value == NULL) {
		if (old_count < 0) return malloc(new_count * size);
		else return calloc(new_count, size);
	} else {
		void *val = realloc(value, new_count * size);

		if (old_count > 0) {
			const long diff = (new_count - old_count);

			if (diff > 0) {
				memset(val + (old_count * size), 0, size * diff);
			}
		}

		return val;
	}
}
