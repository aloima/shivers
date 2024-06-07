#include <shivers.h>
#include <hash.h>

void on_guild_member_add(struct Shivers *shivers, struct Node *guild_node) {
	update_voice_stats(shivers->client, guild_node->key);
}
