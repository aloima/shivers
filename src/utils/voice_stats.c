#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <database.h>
#include <network.h>
#include <shivers.h>
#include <utils.h>
#include <json.h>

struct VoiceStatsChannel *channels = NULL;
unsigned int channel_count = 0;

void update_voice_stats(const struct Client client, const char *guild_id) {
	char database_key[27];
	sprintf(database_key, "%s.vstats", guild_id);

	const jsonresult_t data = database_get(database_key);

	if (data.exist) {
		const jsonelement_t *array = data.value.array;

		for (unsigned int i = 0; i < array->size; ++i) {
			jsonelement_t *object = ((jsonelement_t **) array->value)[i];
			const jsonresult_t name = json_get_val(object, "name");
			const jsonresult_t id = json_get_val(object, "id");

			char path[30] = "/channels/";
			strcat(path, id.value.string);

			unsigned int v = 0;
			bool at_cache = false;

			while (v < channel_count) {
				if (strsame(id.value.string, channels[v].id)) {
					at_cache = true;
					break;
				}

				++v;
			}

			if (!at_cache) {
				++channel_count;
				channels = allocate(channels, -1, channel_count, sizeof(struct VoiceStatsChannel));
				channels[v].id = id.value.string; // no need to allocation, it's in database
				channels[v].name = allocate(NULL, -1, name.element->size + 1, sizeof(char));

				memcpy(channels[v].name, name.value.string, name.element->size + 1);
				prepare_voice_stats_channel_name(&(channels[v].name), guild_id);
			} else {
				char *tmp = allocate(NULL, -1, name.element->size + 1, sizeof(char));
				memcpy(tmp, name.value.string, name.element->size + 1);
				prepare_voice_stats_channel_name(&tmp, guild_id);

				if (strsame(channels[v].name, tmp)) {
					free(tmp);
					continue;
				} else {
					const unsigned int tmp_size = strlen(tmp) + 1;
					channels[v].name = allocate(channels[v].name, -1, tmp_size, sizeof(char));
					memcpy(channels[v].name, tmp, tmp_size);

					free(tmp);
				}
			}

			char body[256];
			sprintf(body, "{\"name\":\"%s\"}", channels[v].name);

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
