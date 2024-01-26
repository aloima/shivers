#include <stdio.h>

#include <shivers.h>
#include <discord.h>

void on_guild_create(struct Client client) {
	char game[22];
	sprintf(game, "%ld servers | " PREFIX "help", get_guilds_cache()->size);
	set_presence(game, 0, "online");
}
