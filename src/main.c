#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/stat.h>
#include <sys/sysmacros.h>

#include <discord.h>
#include <utils.h>

int main(void) {
	struct stat token_entry;
	const char token_stat = stat("token", &token_entry);

	if (token_stat == -1) {
		throw("startup: missing token file");
	} else if (!S_ISREG(token_entry.st_mode)) {
		throw("startup: invalid token entry");
	} else {
		srand(time(NULL));

		FILE *token_file = fopen("token", "r");
		char token[96];

		fgets(token, 96, token_file);
		fclose(token_file);
		connect_gateway(token);

		return EXIT_SUCCESS;
	}
}
