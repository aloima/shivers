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

			char xp_key[50], level_key[53], factor_key[42];
			sprintf(xp_key, "%s.levels.%s.xp", guild_id, user_id);
			sprintf(level_key, "%s.levels.%s.level", guild_id, user_id);
			sprintf(factor_key, "%s.settings.level.factor", guild_id);

			double factor = (database_has(factor_key) ? database_get(factor_key).number : 100.0);
			double xp = (database_has(xp_key) ? database_get(xp_key).number : 0.0);
			double level = (database_has(level_key) ? database_get(level_key).number : 1.0);
			xp += (rand() % 10);

			if (xp >= (level * factor)) {
				xp = 0.0;
				level += 1.0;
			}

			database_set(xp_key, &xp, JSON_NUMBER);
			database_set(level_key, &level, JSON_NUMBER);
		}
	}
}
