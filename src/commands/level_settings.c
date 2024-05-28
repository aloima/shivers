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

	const char *argument = command.arguments[0].name;

	if (strsame(argument, "list")) {
		struct Embed embed = {
			.color = COLOR,
			.title = "Level Settings"
		};

		char key[43], factor_text[24], channel_text[23];
		jsonresult_t data;

		{
			sprintf(key, "%s.settings.level.factor", command.guild_id);
			data = database_get(key);

			if (!data.exist) {
				sprintf(factor_text, "Not set (100)");
			} else {
				sprintf(factor_text, "%d", (int) data.value.number);
			}

			add_field_to_embed(&embed, "Factor", factor_text, true);
		}

		{
			sprintf(key, "%s.settings.level.channel", command.guild_id);
			data = database_get(key);

			if (!data.exist) {
				sprintf(channel_text, "Not set");
			} else {
				sprintf(channel_text, "<#%s>", data.value.string);
			}

			add_field_to_embed(&embed, "Channel", channel_text, true);
		}

		{
			sprintf(key, "%s.settings.level.message", command.guild_id);
			data = database_get(key);

			if (!data.exist) {
				add_field_to_embed(&embed, "Message", "Not set", false);
			} else {
				add_field_to_embed(&embed, "Message", data.value.string, false);
			}

			add_embed_to_message_payload(embed, &(message.payload));
		}

		send_message(client, message);

		free_message_payload(message.payload);
		free(embed.fields);
	} else if (strsame(argument, "set")) {
		unsigned char options_size = command.arguments[0].value.subcommand.argument_size;
		char key[43], response[256] = {0};

		if (options_size != 0) {
			const int last_index = (options_size - 1);
			char temp[34];

			for (unsigned int i = 0; i < options_size; ++i) {
				struct InteractionArgument option = command.arguments[0].value.subcommand.arguments[i];
				char *option_name = option.name;

				if (options_size == 1) {
					sprintf(response, "`%.8s` setting is set.", option_name);
				} else if (i == last_index) {
					sprintf(temp, " and `%.8s` settings are set.", option_name);
					strcat(response, temp);
				} else if (i == 0) {
					sprintf(response, "`%.8s`", option_name);
				} else {
					sprintf(temp, ", `%.8s`", option_name);
					strcat(response, temp);
				}

				if (strsame(option_name, "factor")) {
					double value = option.value.number;
					sprintf(key, "%s.settings.level.factor", command.guild_id);
					database_set(key, &value, JSON_NUMBER);
				} else if (strsame(option_name, "channel")) {
					sprintf(key, "%s.settings.level.channel", command.guild_id);
					database_set(key, json_get_val(option.value.channel, "id").value.string, JSON_STRING);
				} else if (strsame(option_name, "message")) {
					sprintf(key, "%s.settings.level.message", command.guild_id);
					database_set(key, option.value.string.value, JSON_STRING);
				}
			}

			message.payload.content = response;
			send_message(client, message);
		} else {
			message.payload.content = "You must specify an argument to use `/level-settings set` command.";
			send_message(client, message);
		}
	} else if (strsame(argument, "help")) {
		struct Embed embed = {
			.color = COLOR,
			.title = "Level settings"
		};

		add_field_to_embed(&embed, "factor", (
			"This setting is used to determine XP to level up. "
			"For example, if factor setting is 200 and you are 8 level, you need to reach (8 * 200) = 1600 XP for next level. "
			"So, you need (level * factor) XP to level up."
		), false);

		add_field_to_embed(&embed, "channel", (
			"This setting is used to determine "
			"channel to send message when a user leveled up."
		), false);

		add_field_to_embed(&embed, "message", (
			"This setting is used to determine message will be sent when a user level up. This setting has some parameters:\\n"
			"{name} - Username of user who leveled up\\n"
			"{user} - Mention of user who leveled up\\n"
			"{level} - Current level of user who leveled up\\n"
			"{old} - Old level of user who leveled up"
		), false);

		add_embed_to_message_payload(embed, &(message.payload));
		free(embed.fields);

		send_message(client, message);
		free_message_payload(message.payload);
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
		.description = "Sets channel to send message when a user level up.",
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
	},
	(const struct CommandArgument) {
		.name = "help",
		.description = "Describes level settings",
		.type = SUBCOMMAND_ARGUMENT
	}
};

const struct Command level_settings = {
	.execute = execute,
	.name = "level-settings",
	.description = "Base command for level settings of your server",
	.guild_only = true,
	.permissions = (ManageMessages | ManageChannels),
	.args = args,
	.arg_size = sizeof(args) / sizeof(struct CommandArgument)
};
