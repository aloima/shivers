#include <stdio.h>

#include <shivers.h>

void on_force_close() {
	free_cooldowns();
	free_commands();

	puts("\nForce quitting...");
}
