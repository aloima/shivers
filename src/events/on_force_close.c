#include <stdio.h>

#include <vips/vips.h>

#include <shivers.h>

void on_force_close() {
	free_cooldowns();
	free_commands();
	vips_shutdown();

	puts("\nForce quitting...");
}
