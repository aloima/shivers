#include <stdbool.h>

#include <shivers.h>
#include <database.h>
#include <discord.h>
#include <utils.h>
#include <json.h>

static void execute(const struct Client client, const struct InteractionCommand command) {
	struct Message message = {
		.target_type = TARGET_INTERACTION_COMMAND,
		.target = {
			.interaction_command = command,
		},
		.payload = {
			.ephemeral = true
		}
	};

	const char *sc_name = command.arguments[0].name;

	if (strcmp(sc_name, "list") == 0) {
		struct Embed embed = {
			.color = COLOR,
			.title = "Level Settings"
		};

		char key[43], factor_text[24], channel_text[23];

		sprintf(key, "%s.settings.level.factor", command.guild_id);

		if (!database_has(key)) {
			sprintf(factor_text, "Not set (100)");
		} else {
			sprintf(factor_text, "%d", (int) database_get(key).number);
		}

		add_field_to_embed(&embed, "Factor", factor_text, true);

		sprintf(key, "%s.settings.level.channel", command.guild_id);

		if (!database_has(key)) {
			sprintf(channel_text, "Not set");
		} else {
			sprintf(channel_text, "<#%s>", database_get(key).string);
		}

		add_field_to_embed(&embed, "Channel", channel_text, true);

		sprintf(key, "%s.settings.level.message", command.guild_id);

		if (!database_has(key)) {
			add_field_to_embed(&embed, "Message", "Not set", false);
		} else {
			add_field_to_embed(&embed, "Message", database_get(key).string, false);
		}

		add_embed_to_message_payload(embed, &(message.payload));
		send_message(client, message);

		free_message_payload(message.payload);
		free(embed.fields);
	} else if (strcmp(sc_name, "set") == 0) {
		unsigned char sc_arguments_size = command.arguments[0].value.subcommand.argument_size;
		char key[43], response[256] = {0};

		if (sc_arguments_size != 0) {
			const unsigned char last_index = (sc_arguments_size - 1);

			for (unsigned char i = 0; i < sc_arguments_size; ++i) {
				struct InteractionArgument sc_argument = command.arguments[0].value.subcommand.arguments[i];
				char *sc_argument_name = sc_argument.name;

				if (sc_arguments_size == 1) {
					sprintf(response, "`%s` setting is set.", sc_argument_name);
				} else if (i == last_index) {
					sprintf(response, "%s and `%s` settings are set.", response, sc_argument_name);
				} else if (i == 0) {
					sprintf(response, "`%s`", sc_argument_name);
				} else {
					sprintf(response, "%s, `%s`", response, sc_argument_name);
				}

				if (strcmp(sc_argument_name, "factor") == 0) {
					double value = sc_argument.value.number;
					sprintf(key, "%s.settings.level.factor", command.guild_id);
					database_set(key, &value, JSON_NUMBER);
				} else if (strcmp(sc_argument_name, "channel") == 0) {
					sprintf(key, "%s.settings.level.channel", command.guild_id);
					database_set(key, json_get_val(sc_argument.value.channel, "id").value.string, JSON_STRING);
				} else if (strcmp(sc_argument_name, "message") == 0) {
					sprintf(key, "%s.settings.level.message", command.guild_id);
					database_set(key, sc_argument.value.string, JSON_STRING);
				}
			}

			message.payload.content = response;
			send_message(client, message);
		} else {
			message.payload.content = "You must specify an argument to use `/level-settings set` command.";
			send_message(client, message);
		}
	}
}

static const struct CommandArgument set_args[] = {
	(const struct CommandArgument) {
		.name = "factor",
		.description = "Sets factor of level, needed total xp to level up is (factor * level).",
		.type = INTEGER_ARGUMENT,
		.optional = true
	},
	(const struct CommandArgument) {
		.name = "channel",
		.description = "Sets channel to send message when a user leveled up.",
		.type = CHANNEL_ARGUMENT,
		.optional = true
	},
	(const struct CommandArgument) {
		.name = "message",
		.description = "Sets message that will be send when a user leveled up, use {name}, {user} and {level} for variables.",
		.type = STRING_ARGUMENT,
		.optional = true
	}
};

static const struct CommandArgument args[] = {
	(const struct CommandArgument) {
		.name = "set",
		.description = "Sets level settings of your server",
		.type = SUBCOMMAND_ARGUMENT,
		.args = set_args,
		.arg_size = sizeof(set_args) / sizeof(struct CommandArgument)
	},
	(const struct CommandArgument) {
		.name = "list",
		.description = "Lists level settings of your server",
		.type = SUBCOMMAND_ARGUMENT
	}
};

const struct Command level_settings = {
	.execute = execute,
	.name = "level-settings",
	.description = "Base command for level settings of your server",
	.guild_only = true,
	.args = args,
	.arg_size = sizeof(args) / sizeof(struct CommandArgument)
};
