#include <string.h>
#include <stdio.h>

#include <shivers.h>
#include <utils.h>
#include <json.h>

static void execute(Client client, jsonelement_t **message, Split args) {
	const struct Command *commands = get_commands();
	const size_t command_size = get_command_size();

	if (args.size == 1) {
		struct Command command = {0};

		for (size_t i = 0; i < command_size; ++i) {
			if (strcmp(args.data[0], commands[i].name) == 0) {
				command = commands[i];
				break;
			}
		}

		if (command.name != NULL) {
			Embed embed = {0};
			set_embed_author(&embed, command.name, NULL, NULL);
			embed.description = command.description;
			embed.color = COLOR;

			send_embed(client, json_get_val(*message, "channel_id").value.string, embed);
		} else {
			send_content(client, json_get_val(*message, "channel_id").value.string, "Unknown command, please use `help` command to get list of the commands.");
		}
	} else {
		Embed embed = {0};

		char text[4096] = {0};
		sprintf(text, (
			"```\\n"
			"Help page\\n"
			"---------\\n\\n"
		));

		size_t max_length = 0;

		for (size_t i = 0; i < command_size; ++i) {
			const size_t length = strlen(commands[i].name);

			if (length > max_length) {
				max_length = length;
			}
		}

		for (size_t i = 0; i < command_size; ++i) {
			struct Command command = commands[i];

			char blanks[64] = {0};
			char line[256] = {0};
			memset(blanks, ' ', max_length - strlen(command.name));
			sprintf(line, "%s%s | %s\\n", command.name, blanks, command.description);
			strcat(text, line);
		}

		strcat(text, "```");

		embed.color = COLOR;
		embed.description = text;
		send_embed(client, json_get_val(*message, "channel_id").value.string, embed);
	}
}

struct Command help = {
	.execute = execute,
	.name = "help",
	.description = "Sends help page",
	.args = NULL
};
