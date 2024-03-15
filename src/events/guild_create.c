#include <stdio.h>

#include <shivers.h>
#include <discord.h>

void on_guild_create(struct Client client) {
	char custom_status[12];
	sprintf(custom_status, "%u servers", get_guilds_cache()->size);
	set_presence("custom", custom_status, NULL, 4, "online");
}
