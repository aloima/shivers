#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <shivers.h>
#include <discord.h>
#include <utils.h>
#include <json.h>

static void execute(struct Client client, jsonelement_t *message, const struct Split args) {
	struct Message reply = {0};
	const char *channel_id = json_get_val(message, "channel_id").value.string;

	const struct Command *commands = get_commands();
	const size_t command_size = get_command_size();

	if (args.size == 1) {
		struct Command command;
		bool is_found = false;

		const char *query = args.data[0].data;

		for (size_t i = 0; i < command_size; ++i) {
			if (strcmp(query, commands[i].name) == 0) {
				is_found = true;
				memcpy(&command, &commands[i], sizeof(struct Command));
				break;
			}
		}

		if (is_found) {
			struct Embed embed = {0};

			char usage[64];
			char *arguments_description = NULL;

			sprintf(usage, PREFIX "%s", command.name);

			set_embed_author(&embed, command.name, NULL, NULL);
			embed.color = COLOR;
			add_field_to_embed(&embed, "Description", command.description, false);

			if (command.arg_size != 0) {
				struct Join *arguments_text = allocate(NULL, -1, command.arg_size, sizeof(struct Join));

				for (unsigned char i = 0; i < command.arg_size; ++i) {
					const struct CommandArgument argument = command.args[i];
					arguments_text[i].data = allocate(NULL, 0, 192, sizeof(char));

					if (argument.examples) {
						struct Join example_joins[argument.example_size];
						create_join_elements_nz(example_joins, argument.examples, argument.example_size);

						size_t length = calculate_join(example_joins, argument.example_size, ", ");
						char *examples = allocate(NULL, -1, length + 1, sizeof(char));
						join(example_joins, examples, argument.example_size, ", ");

						sprintf((char *) arguments_text[i].data, (
							"**Name**: %s\\n"
							"**Description**: %s\\n"
							"**Is optional?**: %s\\n"
							"**Examples**: %s"
						), argument.name, argument.description, argument.optional ? "Yes" : "No", examples);

						free(examples);
					} else {
						sprintf((char *) arguments_text[i].data, (
							"**Name**: %s\\n"
							"**Description**: %s\\n"
							"**Is optional?**: %s"
						), argument.name, argument.description, argument.optional ? "Yes" : "No");
					}

					if (argument.optional) {
						sprintf(usage, "%s [<%s>]", usage, argument.name);
					} else {
						sprintf(usage, "%s [%s]", usage, argument.name);
					}
				}

				size_t arguments_description_size = calculate_join(arguments_text, command.arg_size, "\\n\\n");
				arguments_description = allocate(NULL, -1, arguments_description_size + 1, sizeof(char));
				join(arguments_text, arguments_description, command.arg_size, "\\n\\n");

				add_field_to_embed(&embed, "Usage", usage, false);
				add_field_to_embed(&embed, "Arguments", arguments_description, false);

				free(arguments_text);
			} else {
				add_field_to_embed(&embed, "Usage", usage, false);
			}


			add_embed_to_message(embed, &reply);
			send_message(client, channel_id, reply);
			free(embed.fields);
			free_message(reply);

			if (command.arg_size != 0) {
				free(arguments_description);
			}
		} else {
			reply.content = "Unknown command, please use `help` command to get list of the commands.";
			send_message(client, channel_id, reply);
		}
	} else {
		struct Embed embed = {0};

		char text[4096];
		strcpy(text, "```\\n");

		unsigned char max_length = 0;

		for (unsigned char i = 0; i < command_size; ++i) {
			const unsigned char length = strlen(commands[i].name);

			if (length > max_length) {
				max_length = length;
			}
		}

		for (size_t i = 0; i < command_size; ++i) {
			const struct Command command = commands[i];

			char blanks[8];
			char line[128];
			unsigned char blank_length = (max_length - strlen(command.name));

			memset(blanks, ' ', blank_length);
			blanks[blank_length] = '\0';

			sprintf(line, "%s%s | %s\\n", command.name, blanks, command.description);
			strcat(text, line);
		}

		strcat(text, "```");

		embed.title = "Help page";
		embed.color = COLOR;
		embed.description = text;

		add_embed_to_message(embed, &reply);
		send_message(client, channel_id, reply);
		free_message(reply);
	}
}

static const struct CommandArgument args[] = {
	(struct CommandArgument) {
		.name = "command",
		.description = "The command that you want to get information",
		.examples = (const char *[]) {"about", "avatar"},
		.example_size = 2,
		.optional = true
	}
};

const struct Command help = {
	.execute = execute,
	.name = "help",
	.description = "Sends help page",
	.args = args,
	.arg_size = sizeof(args) / sizeof(struct CommandArgument)
};
