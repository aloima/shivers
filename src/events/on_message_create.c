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
	}
}
