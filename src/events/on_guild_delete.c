#include <stdio.h>

#include <shivers.h>
#include <discord.h>

void on_guild_delete(struct Client client) {
	char game[12];
	sprintf(game, "%u servers", get_guilds_cache()->size);
	set_presence(game, 0, "online");
}
