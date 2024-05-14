#include <shivers.h>

void on_presence_update(struct Client client, struct Guild *guild) {
	update_voice_stats(client, guild->id);
}
