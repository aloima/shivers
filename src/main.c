#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(_WIN32)
	#include <winsock2.h>
	#include <windows.h>
#endif

#include <sys/stat.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <discord.h>
#include <utils.h>

int main(void) {
	struct stat token_entry;
	const char token_stat = stat("token", &token_entry);

	#if defined(_WIN32)
		WSADATA wsa;
		WSAStartup(MAKEWORD(2, 2), &wsa);
	#endif

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

		#if defined(_WIN32)
			WSACleanup();
		#endif

		return EXIT_SUCCESS;
	}
}
