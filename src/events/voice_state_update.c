#include <shivers.h>

void on_voice_state_update(struct Client client, struct Guild *guild) {
	update_voice_stats(client, guild->id);
}
