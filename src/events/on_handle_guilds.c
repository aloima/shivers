#include <stdio.h>

#include <shivers.h>
#include <discord.h>

void on_handle_guilds(struct Client client) {
	puts("Handled all guilds.");

	char game[12];
	sprintf(game, "%u servers", get_guilds_cache()->size);
	set_presence(game, 0, "online");
}
