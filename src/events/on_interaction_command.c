#include <string.h>

#include <shivers.h>
#include <database.h>
#include <utils.h>
#include <json.h>

void on_interaction_command(struct Client client, const struct InteractionCommand interaction_command) {
	const unsigned short command_size = get_command_size();
	const struct Command *commands = get_commands();
	const char *user_id = json_get_val(interaction_command.user, "id").value.string;

	for (unsigned short i = 0; i < command_size; ++i) {
		const struct Command command = commands[i];

		if (strcmp(interaction_command.name, command.name) == 0) {
			run_with_cooldown(user_id, command.execute, client, interaction_command);
			break;
		}
	}
}
