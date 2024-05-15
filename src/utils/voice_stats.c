#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <database.h>
#include <network.h>
#include <shivers.h>
#include <utils.h>
#include <json.h>

void update_voice_stats(const struct Client client, const char *guild_id) {
	char database_key[27];
	sprintf(database_key, "%s.vstats", guild_id);

	if (database_has(database_key)) {
		jsonelement_t *array = database_get(database_key).array;

		for (unsigned short i = 0; i < array->size; ++i) {
			jsonelement_t *object = ((jsonelement_t **) array->value)[i];
			const jsonresult_t json_name = json_get_val(object, "name");

			const char *id = json_get_val(object, "id").value.string;
			char *name = allocate(NULL, -1, json_name.element->size + 1, sizeof(char));

			memcpy(name, json_name.value.string, json_name.element->size + 1);
			prepare_voice_stats_channel_name(&name, guild_id);

			char path[30];
			sprintf(path, "/channels/%s", id);

			char body[256];
			sprintf(body, (
				"{"
					"\"name\":\"%s\""
				"}"
			), name);

			free(name);

			response_free(api_request(client.token, path, "PATCH", body, NULL));
		}
	}
}

void prepare_voice_stats_channel_name(char **channel_name, const char *guild_id) {
	const struct Guild *guild = get_guild_from_cache(guild_id);

	char members[(guild->member_count / 10) + 2];
	char online[(guild->online_count / 10) + 2];
	char bots[(guild->bot_count / 10) + 2];
	char at_voice[(guild->member_at_voice_count / 10) + 2];

	sprintf(members, "%lld", guild->member_count);
	sprintf(online, "%lld", guild->online_count);
	sprintf(bots, "%lld", guild->bot_count);
	sprintf(at_voice, "%lld", guild->member_at_voice_count);

	strreplace(channel_name, "{members}", members);
	strreplace(channel_name, "{online}", online);
	strreplace(channel_name, "{bots}", bots);
	strreplace(channel_name, "{atVoice}", at_voice);
}
