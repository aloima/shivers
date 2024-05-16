#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(_WIN32)
	#include <winsock2.h>
	#include <windows.h>
#endif

#include <sys/stat.h>

#include <discord.h>
#include <utils.h>

int main(void) {
	struct stat token_entry;

	#if defined(_WIN32)
		WSADATA wsa;
		WSAStartup(MAKEWORD(2, 2), &wsa);
	#endif

	if (stat("token", &token_entry) == -1) {
		throw("startup: missing token file");
	} else if (!S_ISREG(token_entry.st_mode)) {
		throw("startup: invalid token entry");
	} else {
		srand(time(NULL));

		FILE *bot_token_file = fopen("token", "r");
		char bot_token[84];

		fgets(bot_token, 84, bot_token_file);
		fclose(bot_token_file);
		connect_gateway(bot_token, "wss://gateway.discord.gg", (
			(1 << 0) | // GUILDS
			(1 << 1) | // GUILD_MEMBERS
			(1 << 7) | // GUILD_VOCE_STATES
			(1 << 8) | // GUILD_PRESENCES
			(1 << 9)   // GUILD_MESSAGES
		));

		#if defined(_WIN32)
			WSACleanup();
		#endif

		return EXIT_SUCCESS;
	}
}
