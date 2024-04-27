#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <shivers.h>
#include <discord.h>
#include <database.h>
#include <json.h>
#include <utils.h>

static void execute(const struct Client client, const struct InteractionCommand command) {
	struct Embed embed = {
		.color = COLOR
	};

	struct Message message = {
		.target_type = TARGET_INTERACTION_COMMAND,
		.target = {
			.interaction_command = command
		},
		.payload = {
			.ephemeral = true
		}
	};

	const char *main_argument = command.arguments[0].name;

	if (strsame(main_argument, "help")) {
		embed.title = "Voice Stats Help Page";
		embed.description = (
			"To add a voice channel using voice stats system, use `/vstats add {name}`.\\n"
			"To delete a voice channel, use `/vstats delete {id}`.\\n"
			"To list voice stats channels with their IDs, use `vstats list` command.\\n\\n"
			"You can use some arguments in voice stats channels' name:\\n"
			"{members} - Displays total members in the server\\n"
			"{online} - Displays total online members in the server\\n"
			"{bots} - Displays total bots in the server"
		);

		add_embed_to_message_payload(embed, &(message.payload));
	} else if (strsame(main_argument, "list")) {
		char description[4096];
		description[0] = 0;

		char database_key[27];
		sprintf(database_key, "%s.vstats", command.guild_id);

		if (database_has(database_key)) {
			jsonelement_t *data = database_get(database_key).array;
			char line[256];

			for (unsigned int i = 0; i < data->size; ++i) {
				char id_key[6], name_key[8];
				sprintf(id_key, "%d.id", i);
				sprintf(name_key, "%d.name", i);

				char *id = json_get_val(data, id_key).value.string;
				char *name = json_get_val(data, name_key).value.string;

				sprintf(line, "%s | <#%s> | %s\\n", id, id, name);
				strcat(description, line);
			}

			embed.title = "Voice Stats Channel List";
			embed.description = description;

			add_embed_to_message_payload(embed, &(message.payload));
		} else {
			strcpy(description, "There is no channel. Use `/vstats help` to learn how to add.");
		}
	} else if (strsame(main_argument, "add")) {
		char url[37];
		sprintf(url, "/guilds/%s/channels", command.guild_id);

		char database_key[27];
		sprintf(database_key, "%s.vstats", command.guild_id);

		const char *name = command.arguments[0].value.subcommand.arguments[0].value.string;
		const unsigned int name_length = strlen(name);

		char *channel_name = allocate(NULL, -1, (name_length + 1), sizeof(char));
		memcpy(channel_name, name, name_length + 1);

		char members[2] = "0";
		char online[2] = "1";
		char bots[2] = "2";

		strreplace(&channel_name, "{members}", members);
		strreplace(&channel_name, "{online}", online);
		strreplace(&channel_name, "{bots}", bots);

		char request_payload[4096];
		sprintf(request_payload, (
			"{"
				"\"name\":\"%s\","
				"\"type\":2"
			"}"
		), channel_name);

		struct Response response = api_request(client.token, url, "POST", request_payload, NULL);
		jsonelement_t *response_data = json_parse((char *) response.data);

		jsonelement_t *database_data = create_empty_json_element(false);
		json_set_val(database_data, "id", json_get_val(response_data, "id").value.string, JSON_STRING);
		json_set_val(database_data, "name", (char *) name, JSON_STRING);
		json_free(response_data, false);

		database_push(database_key, database_data, JSON_OBJECT);
		json_free(database_data, false);

		message.payload.content = "Channel is created.";
	}

	send_message(client, message);
	free_message_payload(message.payload);
}

static const struct CommandArgument add_args[] = {
	(const struct CommandArgument) {
		.name = "name",
		.description = "Sets voice stats channel name.",
		.type = STRING_ARGUMENT,
		.optional = false
	}
};

static const struct CommandArgument delete_args[] = {
	(const struct CommandArgument) {
		.name = "id",
		.description = "Specifies voice stats channel ID which you want to delete.",
		.type = STRING_ARGUMENT,
		.optional = false
	}
};

static const struct CommandArgument args[] = {
	(const struct CommandArgument) {
		.name = "add",
		.description = "Adds a voice stats channel",
		.type = SUBCOMMAND_ARGUMENT,
		.args = add_args,
		.arg_size = sizeof(add_args) / sizeof(struct CommandArgument)
	},
	(const struct CommandArgument) {
		.name = "delete",
		.description = "Deletes a voice stats channel",
		.type = SUBCOMMAND_ARGUMENT,
		.args = delete_args,
		.arg_size = sizeof(delete_args) / sizeof(struct CommandArgument)
	},
	(const struct CommandArgument) {
		.name = "list",
		.description = "Lists voice stats channels",
		.type = SUBCOMMAND_ARGUMENT
	},
	(const struct CommandArgument) {
		.name = "help",
		.description = "Describes voice stats system",
		.type = SUBCOMMAND_ARGUMENT
	}
};

const struct Command vstats = {
	.execute = execute,
	.name = "vstats",
	.description = "Sets up voice stats",
	.guild_only = true,
	.permissions = (ManageChannels),
	.args = args,
	.arg_size = sizeof(args) / sizeof(struct CommandArgument)
};
