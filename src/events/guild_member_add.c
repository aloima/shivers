#include <shivers.h>

void on_guild_member_add(struct Client client, struct Guild *guild) {
	update_voice_stats(client, guild->id);
}
