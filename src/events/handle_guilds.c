#include <stdio.h>

#include <shivers.h>
#include <discord.h>

void on_handle_guilds(struct Client client) {
	puts("Handled all guilds.");

	for (unsigned int i = 0; i < client.guilds->size; ++i) {
		struct Node *guild_node = client.guilds->nodes[i];

		while (guild_node) {
			update_voice_stats(client, guild_node->key);
			guild_node = guild_node->next;
		}
	}

	puts("Updated voice stats of all servers.");

	char custom_status[12];
	sprintf(custom_status, "%u servers", client.guilds->length);
	set_presence("custom", custom_status, NULL, 4, "online");
}
