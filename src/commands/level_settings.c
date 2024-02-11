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

		char factor_key[42], factor_text[24];
		sprintf(factor_key, "%s.settings.level.factor", command.guild_id);

		if (!database_has(factor_key)) {
			sprintf(factor_text, "Not set (100)");
		} else {
			sprintf(factor_text, "%d", (int) database_get(factor_key).number);
		}

		add_field_to_embed(&embed, "Factor", factor_text, true);

		add_embed_to_message_payload(embed, &(message.payload));
		send_message(client, message);

		free_message_payload(message.payload);
		free(embed.fields);
	} else if (strcmp(sc_name, "set") == 0) {
		unsigned char sc_args_size = command.arguments[0].value.subcommand.argument_size;
		char factor_key[42];

		if (sc_args_size != 0) {
			for (unsigned char i = 0; i < sc_args_size; ++i) {
				struct InteractionArgument sc_argument = command.arguments[0].value.subcommand.arguments[i];
				const char *sc_argument_name = sc_argument.name;

				if (strcmp(sc_argument_name, "factor") == 0) {
					double value = sc_argument.value.number;
					sprintf(factor_key, "%s.settings.level.factor", command.guild_id);
					database_set(factor_key, &value, JSON_NUMBER);
				}
			}

			message.payload.content = "Set!";
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
	.args = args,
	.arg_size = sizeof(args) / sizeof(struct CommandArgument)
};
