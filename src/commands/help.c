#include <string.h>
#include <stdio.h>

#include <shivers.h>
#include <discord.h>
#include <utils.h>
#include <json.h>

static void execute(struct Client client, jsonelement_t **message, Split args) {
	struct Message reply = {0};
	const char *channel_id = json_get_val(*message, "channel_id").value.string;

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
			struct Embed embed = {0};
			set_embed_author(&embed, command.name, NULL, NULL);
			embed.description = command.description;
			embed.color = COLOR;

			add_embed_to_message(embed, &reply);
			send_message(client, channel_id, reply);
			free_message(reply);
		} else {
			reply.content = "Unknown command, please use `help` command to get list of the commands.";
			send_message(client, channel_id, reply);
		}
	} else {
		struct Embed embed = {0};

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
			const struct Command command = commands[i];

			char blanks[64] = {0};
			char line[256] = {0};
			memset(blanks, ' ', max_length - strlen(command.name));
			sprintf(line, "%s%s | %s\\n", command.name, blanks, command.description);
			strcat(text, line);
		}

		strcat(text, "```");

		embed.color = COLOR;
		embed.description = text;

		add_embed_to_message(embed, &reply);
		send_message(client, channel_id, reply);
		free_message(reply);
	}
}

const struct Command help = {
	.execute = execute,
	.name = "help",
	.description = "Sends help page",
	.args = NULL
};
