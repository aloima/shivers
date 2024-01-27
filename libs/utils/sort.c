#include <utils.h>

static void compare_elements(long *a, long *b) {
	if (*b > *a) {
		long temp = *a;
		*a = *b;
		*b = temp;
	}
}

void sort(long *data, const unsigned long size) {
	for (unsigned long a = 0; a < size - 1; ++a) {
		for (unsigned long i = 0; i < size - a - 1; ++i) {
			compare_elements(&data[i], &data[i + 1]);
		}
	}
}
