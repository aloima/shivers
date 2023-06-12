#include <stdio.h>
#include <stdlib.h>

#include <discord.h>

int main(void) {
	const char *token = getenv("TOKEN");

	if (token == NULL) {
		fprintf(stderr, "startup: not provided token\n");
		return EXIT_FAILURE;
	} else {
		connect_gateway(token);

		return EXIT_SUCCESS;
	}
}
