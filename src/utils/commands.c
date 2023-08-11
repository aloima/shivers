#include <string.h>

#include <shivers.h>
#include <utils.h>

static struct Command *commands = NULL;
static size_t command_size = 0;

void setup_commands() {
	struct Command command_array[] = {about, avatar, github, help, wikipedia};
	command_size = 5;
	commands = allocate(NULL, 0, command_size, sizeof(struct Command));
	memcpy(commands, command_array, sizeof(command_array));
}

const struct Command *get_commands() {
	return commands;
}

const size_t get_command_size() {
	return command_size;
}
