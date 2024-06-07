#include <shivers.h>
#include <hash.h>

void on_voice_state_update(struct Shivers *shivers, struct Node *guild_node) {
	update_voice_stats(shivers->client, guild_node->key);
}
