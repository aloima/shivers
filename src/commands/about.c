#include <string.h>
#include <stdlib.h>

#include <sys/resource.h>

#include <shivers.h>
#include <discord.h>
#include <utils.h>

void about(Client client, JSONElement **message, Split args) {
	Embed embed;
	memset(&embed, 0, sizeof(Embed));

	struct rusage r_usage;
	char memory_usage[32] = {0};

	getrusage(RUSAGE_SELF, &r_usage);
	sprintf(memory_usage, "%.2f MB", (double) r_usage.ru_maxrss / 1024);

	char guilds[8];
	sprintf(guilds, "%ld", get_cache_size(get_guilds_cache()));

	embed.color = COLOR;

	add_field_to_embed(&embed, "Developer", "<@840217542400409630>", true);
	add_field_to_embed(&embed, "Memory usage", memory_usage, true);
	add_field_to_embed(&embed, "Guilds", guilds, true);

	send_embed(client, json_get_val(*message, "channel_id").value.string, embed);
	free(embed.fields);
}
