#include <string.h>

#include <shivers.h>
#include <utils.h>

static struct Command *commands = NULL;
static unsigned short command_size = 0;

void setup_commands() {
	const struct Command command_array[] = {about, avatar, github, help, level, wikipedia};
	command_size = sizeof(command_array) / sizeof(struct Command);

	commands = allocate(NULL, -1, command_size, sizeof(struct Command));
	memcpy(commands, command_array, sizeof(command_array));
}

void free_commands() {
	free(commands);
}

const struct Command *get_commands() {
	return commands;
}

const unsigned short get_command_size() {
	return command_size;
}
