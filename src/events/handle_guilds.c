#include <stdio.h>

#include <shivers.h>
#include <discord.h>

void on_handle_guilds(struct Client client) {
	puts("Handled all guilds.");

	const unsigned int guild_count = get_guild_count();
	struct Guild *guilds = get_guilds();

	for (unsigned int i = 0; i < guild_count; ++i) {
		update_voice_stats(client, guilds[i].id);
	}

	puts("Updated voice stats of all servers.");

	char custom_status[12];
	sprintf(custom_status, "%u servers", guild_count);
	set_presence("custom", custom_status, NULL, 4, "online");
}
