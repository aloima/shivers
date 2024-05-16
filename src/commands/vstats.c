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

	const char *operation = command.arguments[0].name;

	if (strsame(operation, "help")) {
		embed.title = "Voice Stats Help Page";
		embed.description = (
			"To add a voice channel using voice stats system, use `/vstats add {name}`.\\n"
			"To delete a voice channel, use `/vstats delete {id}`.\\n"
			"To list voice stats channels with their IDs, use `vstats list` command.\\n\\n"
			"You can use some arguments in voice stats channels' name:\\n"
			"{members} - Displays total members in the server\\n"
			"{online} - Displays total online members in the server\\n"
			"{bots} - Displays total bots in the server\\n"
			"{atVoice} - Displays members at a voice channel"
		);

		add_embed_to_message_payload(embed, &(message.payload));
	} else if (strsame(operation, "list")) {
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

				const char *id = json_get_val(data, id_key).value.string;
				const char *name = json_get_val(data, name_key).value.string;

				sprintf(line, "%s | <#%s> | %s\\n", id, id, name);
				strcat(description, line);
			}

			embed.title = "Voice Stats Channel List";
			embed.description = description;

			add_embed_to_message_payload(embed, &(message.payload));
		} else {
			memcpy(description, "There is no channel. Use `/vstats help` to learn how to add.", 61);
			embed.description = description;

			add_embed_to_message_payload(embed, &(message.payload));
		}
	} else if (strsame(operation, "add")) {
		char path[37];
		sprintf(path, "/guilds/%s/channels", command.guild_id);

		const char *name = command.arguments[0].value.subcommand.arguments[0].value.string;
		const unsigned int name_length = strlen(name);

		if (name_length > 96) {
			message.payload.content = "The length of a voice stats channel name must be `lower than or equal to 96`.";
			send_message(client, message);
			return;
		}

		char *channel_name = allocate(NULL, -1, name_length + 1, sizeof(char));
		memcpy(channel_name, name, name_length + 1);

		prepare_voice_stats_channel_name(&channel_name, command.guild_id);

		char request_payload[256];
		sprintf(request_payload, (
			"{"
				"\"name\":\"%s\","
				"\"type\":2"
			"}"
		), channel_name);
		free(channel_name);

		struct Response response = api_request(client.token, path, "POST", request_payload, NULL);
		jsonelement_t *response_data = json_parse((char *) response.data);
		response_free(response);

		if (json_get_val(response_data, "code").value.number == 50013) {
			json_free(response_data, false);

			message.payload.content = MISSING_MANAGE_CHANNELS;
			send_message(client, message);
			return;
		}

		jsonelement_t *database_data = create_empty_json_element(false);
		json_set_val(database_data, "id", json_get_val(response_data, "id").value.string, JSON_STRING);
		json_set_val(database_data, "name", (char *) name, JSON_STRING);
		json_free(response_data, false);

		char database_key[27];
		sprintf(database_key, "%s.vstats", command.guild_id);

		database_push(database_key, database_data, JSON_OBJECT);
		json_free(database_data, false);

		message.payload.content = "Channel is created.";
	} else if (strsame(operation, "delete")) {
		char database_key[27];
		sprintf(database_key, "%s.vstats", command.guild_id);

		const char *input = command.arguments[0].value.subcommand.arguments[0].value.string;
		bool deleted = false;

		if (database_has(database_key)) {
			jsonelement_t *data = database_get(database_key).array;

			for (unsigned int i = 0; i < data->size; ++i) {
				char id_key[6];
				sprintf(id_key, "%d.id", i);

				const char *id = json_get_val(data, id_key).value.string;

				if (strsame(id, input)) {
					char database_deletion_key[30], path[30];
					sprintf(database_deletion_key, "%s.vstats.%d", command.guild_id, i);
					sprintf(path, "/channels/%s", id);

					database_delete(database_deletion_key);
					response_free(api_request(client.token, path, "DELETE", NULL, NULL));
					deleted = true;

					message.payload.content = "Channel is deleted.";
					break;
				}
			}

			if (!deleted) {
				message.payload.content = "There is no channel that has this ID.";
			}
		}
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
