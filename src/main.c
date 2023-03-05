#include <stdio.h>
#include <stdlib.h>

int main() {
	if (getenv("TOKEN") == NULL) {
		fprintf(stderr, "startup: not provided token\n");
		return EXIT_FAILURE;
	} else {
		return EXIT_SUCCESS;
	}
}
