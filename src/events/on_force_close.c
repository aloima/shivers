#include <stdio.h>

#include <database.h>
#include <shivers.h>

void on_force_close() {
	free_cooldowns();
	free_commands();
	database_save();
	database_destroy();

	puts("\nForce quitting...");
}
