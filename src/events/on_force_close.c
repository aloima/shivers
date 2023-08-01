#include <shivers.h>

void on_force_close() {
	free_cooldown_memory();

	puts("\nForce quitting...");
}
