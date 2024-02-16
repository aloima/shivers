#include <shivers.h>
#include <database.h>
#include <utils.h>
#include <json.h>

void on_message_create(struct Client client, jsonelement_t *message) {
	const bool is_webhook = json_get_val(message, "webhook_id").exist;

	if (!is_webhook) {
		const jsonresult_t author_bot = json_get_val(message, "author.bot");
		const jsonresult_t guild_id_element = json_get_val(message, "guild_id");

		if ((!author_bot.exist || author_bot.value.boolean == false) && (guild_id_element.exist && guild_id_element.type != JSON_NULL)) {
			const char *user_id = json_get_val(message, "author.id").value.string;
			const char *guild_id = guild_id_element.value.string;

			char xp_key[50], level_key[53], factor_key[42], channel_key[43], message_key[43];
			sprintf(xp_key, "%s.levels.%s.xp", guild_id, user_id);
			sprintf(level_key, "%s.levels.%s.level", guild_id, user_id);
			sprintf(factor_key, "%s.settings.level.factor", guild_id);
			sprintf(channel_key, "%s.settings.level.channel", guild_id);
			sprintf(message_key, "%s.settings.level.message", guild_id);

			double xp = (database_has(xp_key) ? database_get(xp_key).number : 0.0);
			double level = (database_has(level_key) ? database_get(level_key).number : 1.0);

			const double factor = (database_has(factor_key) ? database_get(factor_key).number : 100.0);
			const double bound = (level * factor);
			xp += (rand() % 10);

			if (xp >= bound) {
				xp -= bound;
				level += 1.0;

				database_set(level_key, &level, JSON_NUMBER);

				if (database_has(channel_key) && database_has(message_key)) {
					char *level_message = database_get(message_key).string;

					const struct Message message = {
						.target_type = TARGET_CHANNEL,
						.target = {
							.channel_id = database_get(channel_key).string
						},
						.payload = {
							.content = level_message
						}
					};

					const unsigned short status = send_message(client, message);

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
