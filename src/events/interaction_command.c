#include <shivers.h>
#include <utils.h>
#include <json.h>

void on_interaction_command(struct Client client, const struct InteractionCommand interaction_command) {
	const unsigned char command_size = get_command_size();
	const struct Command *commands = get_commands();
	const char *user_id = json_get_val(interaction_command.user, "id").value.string;

	for (unsigned char i = 0; i < command_size; ++i) {
		const struct Command command = commands[i];

		if (strsame(interaction_command.name, command.name)) {
			run_with_cooldown(user_id, command.execute, client, interaction_command);
			return;
		}
	}
}
