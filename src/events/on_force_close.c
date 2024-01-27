#include <stdio.h>

#if defined(_WIN32)
	#include <winsock2.h>
	#include <windows.h>
#endif

#include <database.h>
#include <shivers.h>

void on_force_close() {
	free_cooldowns();
	free_commands();
	database_save();
	database_destroy();

	#if defined(_WIN32)
		WSACleanup();
	#endif

	puts("\nForce quitting...");
}
