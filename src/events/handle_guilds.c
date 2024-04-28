#include <stdio.h>

#include <shivers.h>
#include <discord.h>

void on_handle_guilds(struct Client client) {
	puts("Handled all guilds.");

	char custom_status[12];
	sprintf(custom_status, "%u servers", get_guild_count());
	set_presence("custom", custom_status, NULL, 4, "online");
}
