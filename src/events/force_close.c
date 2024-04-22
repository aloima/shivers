#include <stdio.h>

#if defined(_WIN32)
	#include <winsock2.h>
	#include <windows.h>
#endif

#include <database.h>
#include <shivers.h>
#include <png.h>

void on_force_close() {
	free_cooldowns();
	puts("\nFree'd cooldowns.");

	free_commands();
	puts("Free'd commands.");

	database_save();
	database_destroy();
	puts("Database is saved and destroyed.");

	free_fonts();
	puts("Free'd fonts.");

	#if defined(_WIN32)
		WSACleanup();
		puts("Cleaned up Windows API.");
	#endif

	puts("Force quitting...");
}
