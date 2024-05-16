#include <string.h>
#include <stdlib.h>

#include <discord.h>
#include <utils.h>

static struct Guild *guilds = NULL;
static unsigned int guild_count = 0;

void clear_guilds() {
	for (unsigned int i = 0; i < guild_count; ++i) {
		free(guilds[i].id);

		for (unsigned long long s = 0; s < guilds[i].online_count; ++s) {
			free(guilds[i].online_members[s]);
		}

		for (unsigned long long s = 0; s < guilds[i].member_at_voice_count; ++s) {
			free(guilds[i].members_at_voice[s]);
		}

		free(guilds[i].online_members);
		free(guilds[i].members_at_voice);
	}

	free(guilds);
	guilds = NULL;
	guild_count = 0;
}

unsigned int get_guild_count() {
	return guild_count;
}

struct Guild *get_guilds() {
	return guilds;
}

void add_guild_to_cache(struct Guild guild) {
	++guild_count;
	guilds = allocate(guilds, -1, guild_count, sizeof(struct Guild));
	memcpy(guilds + guild_count - 1, &guild, sizeof(struct Guild));
}

struct Guild *get_guild_from_cache(const char *id) {
	for (unsigned int i = 0; i < guild_count; ++i) {
		struct Guild *guild = &(guilds[i]);

		if (strsame(guild->id, id)) {
			return guild;
		}
	}

	return NULL;
}

void remove_guild_from_cache(const char *id) {
	for (unsigned int i = 0; i < guild_count; ++i) {
		if (strsame(guilds[i].id, id)) {
			free(guilds[i].id);

			for (unsigned long long s = 0; s < guilds[i].online_count; ++s) {
				free(guilds[i].online_members[s]);
			}

			for (unsigned long long s = 0; s < guilds[i].member_at_voice_count; ++s) {
				free(guilds[i].members_at_voice[s]);
			}

			free(guilds[i].online_members);
			free(guilds[i].members_at_voice);

			for (unsigned int m = (i + 1); m < guild_count; ++m) {
				memcpy(guilds + m - 1, guilds + m, sizeof(struct Guild));
			}

			--guild_count;
			guilds = allocate(guilds, -1, guild_count, sizeof(struct Guild));
			return;
		}
	}
}
