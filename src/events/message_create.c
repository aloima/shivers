#include <stdio.h>

#include <shivers.h>
#include <database.h>
#include <utils.h>
#include <json.h>

void on_message_create(struct Shivers *shivers, jsonelement_t *message) {
	const bool is_webhook = json_get_val(message, "webhook_id").exist;

	if (!is_webhook) {
		const jsonresult_t author_bot = json_get_val(message, "author.bot");
		const jsonresult_t json_guild_id = json_get_val(message, "guild_id");

		if ((!author_bot.exist || author_bot.value.boolean == false) && (json_guild_id.exist && json_guild_id.element->type != JSON_NULL)) {
			const char *user_id = json_get_val(message, "author.id").value.string;
			const char *guild_id = json_guild_id.value.string;

			char xp_key[50], level_key[53], factor_key[42], channel_key[43], message_key[43];
			sprintf(xp_key, "%s.levels.%s.xp", guild_id, user_id);
			sprintf(level_key, "%s.levels.%s.level", guild_id, user_id);
			sprintf(factor_key, "%s.settings.level.factor", guild_id);
			sprintf(channel_key, "%s.settings.level.channel", guild_id);
			sprintf(message_key, "%s.settings.level.message", guild_id);

			const jsonresult_t xp_data = database_get(xp_key);
			const jsonresult_t level_data = database_get(level_key);
			const jsonresult_t factor_data = database_get(factor_key);

			double xp = (xp_data.exist ? xp_data.value.number : 0.0);
			double level = (level_data.exist ? level_data.value.number : 1.0);

			const double factor = (factor_data.exist ? factor_data.value.number : 100.0);
			const double bound = (level * factor);
			xp += (rand() % 10);

			if (xp >= bound) {
				xp -= bound;
				level += 1.0;

				database_set(level_key, &level, JSON_NUMBER);

				const jsonresult_t channel_data = database_get(channel_key);
				const jsonresult_t message_data = database_get(message_key);

				if (channel_data.element && message_data.exist) {
					char *level_message = message_data.value.string;
					char *level_message_dup = allocate(NULL, -1, strlen(level_message) + 1, sizeof(char));
					strcpy(level_message_dup, level_message);

					char *username = json_get_val(message, "author.username").value.string;
					char mention[23], old_level[4], new_level[4];
					sprintf(mention, "<@%s>", user_id);
					sprintf(old_level, "%.0f", level - 1.0);
					sprintf(new_level, "%.0f", level);
					strreplace(&level_message_dup, "{name}", username);
					strreplace(&level_message_dup, "{user}", mention);
					strreplace(&level_message_dup, "{old}", old_level);
					strreplace(&level_message_dup, "{level}", new_level);

					const struct Message message = {
						.target_type = TARGET_CHANNEL,
						.target = {
							.channel_id = channel_data.value.string
						},
						.payload = {
							.content = level_message_dup
						}
					};

					const unsigned short status = send_message(shivers->client, message);
					free(level_message_dup);

					switch (status) {
						case 404:
							database_delete(channel_key);
							break;

						case 403:
							// no permissions
							break;
					}
				}
			}

			database_set(xp_key, &xp, JSON_NUMBER);
		}
	}
}
