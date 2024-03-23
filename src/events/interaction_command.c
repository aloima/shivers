#include <stdbool.h>

#include <shivers.h>
#include <database.h>
#include <utils.h>
#include <json.h>

void on_interaction_command(struct Client client, const struct InteractionCommand interaction_command) {
	const unsigned char command_size = get_command_size();
	const struct Command *commands = get_commands();
	const char *user_id = json_get_val(interaction_command.user, "id").value.string;
	const struct Message message = {
		.target_type = TARGET_INTERACTION_COMMAND,
		.target = {
			.interaction_command = interaction_command
		},
		.payload = {
			.content = "You cannot use this command in direct messages.",
			.ephemeral = true
		}
	};

	for (unsigned char i = 0; i < command_size; ++i) {
		const struct Command command = commands[i];

		if (strsame(interaction_command.name, command.name)) {
			if (!interaction_command.guild_id && command.guild_only) {
				send_message(client, message);
				break;
			}

			run_with_cooldown(user_id, command.execute, client, interaction_command);
			break;
		}
	}
}
