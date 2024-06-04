#include <shivers.h>
#include <hash.h>

void on_voice_state_update(struct Client client, struct Node *guild_node) {
	update_voice_stats(client, guild_node->key);
}
