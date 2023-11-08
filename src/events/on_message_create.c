#include <string.h>

#include <shivers.h>
#include <database.h>
#include <utils.h>
#include <json.h>

void on_message_create(struct Client client, jsonelement_t *message) {
	const bool is_webhook = json_get_val(message, "webhook_id").exist;

	if (!is_webhook) {
		const char *user_id = json_get_val(message, "author.id").value.string;
		const jsonresult_t author_bot = json_get_val(message, "author.bot");

		if (!author_bot.exist || author_bot.value.boolean == false) {
			char xp_key[22], level_key[25];
			sprintf(xp_key, "%s.xp", user_id);
			sprintf(level_key, "%s.level", user_id);

			double xp = database_has(xp_key) ? database_get(xp_key).number : 0.0;
			double level = database_has(level_key) ? database_get(level_key).number : 1.0;
			xp += (rand() % 10);

			if (xp >= (level * 100.0)) {
				xp = 0.0;
				level += 1.0;
			}

			database_set(xp_key, &xp, JSON_NUMBER);
			database_set(level_key, &level, JSON_NUMBER);
		}

		const char *content = json_get_val(message, "content").value.string;
		const unsigned char prefix_length = strlen(PREFIX);

		if (content != NULL && strncmp(content, PREFIX, prefix_length) == 0) {
			Split splitted = split(content + prefix_length, " ");
			Split args = {
				.data = splitted.data + 1,
				.size = splitted.size - 1
			};

			const char *input = splitted.data[0];

			const unsigned short command_size = get_command_size();
			const struct Command *commands = get_commands();

			for (unsigned short i = 0; i < command_size; ++i) {
				const struct Command command = commands[i];

				if (strcmp(input, command.name) == 0) {
					run_with_cooldown(user_id, command.execute, client, message, args);
					break;
				}
			}

			split_free(&splitted);
		}
	}
}
