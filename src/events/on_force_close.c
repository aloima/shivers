#include <stdio.h>

#include <shivers.h>

void on_force_close() {
	free_cooldown_memory();
	free_commands();

	puts("\nForce quitting...");
}
