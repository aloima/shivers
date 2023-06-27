#include <stdio.h>
#include <stdlib.h>

#include <sys/stat.h>
#include <sys/sysmacros.h>

#include <discord.h>

int main(void) {
	struct stat token_entry;
	int token = stat("token", &token_entry);

	if (token == -1) {
		fprintf(stderr, "startup: missing token file\n");
		return EXIT_FAILURE;
	} else if (!S_ISREG(token_entry.st_mode)) {
		fprintf(stderr, "startup: invalid token entry\n");
		return EXIT_FAILURE;
	} else {
		FILE *token_file = fopen("token", "r");
		char token[256] = {0};

		fgets(token, 256, token_file);
		fclose(token_file);
		connect_gateway(token);

		return EXIT_SUCCESS;
	}
}
