#include <string.h>
#include <stdlib.h>

#include <sys/resource.h>

#include <shivers.h>
#include <discord.h>
#include <utils.h>

void about(Client client, JSONElement **message) {
	Embed embed;
	memset(&embed, 0, sizeof(Embed));

	struct rusage r_usage;
	char memory_usage[32] = {0};

	getrusage(RUSAGE_SELF, &r_usage);
	sprintf(memory_usage, "%.2f MB", (double) r_usage.ru_maxrss / 1024);

	char guild_count[8];
	sprintf(guild_count, "%ld", get_cache_size(get_guilds_cache()));

	embed.field_size = 3;
	embed.fields = allocate(NULL, 3, sizeof(EmbedField));
	embed.fields[0] = (EmbedField) {
		.name = "Developer",
		.value = "<@840217542400409630>",
		.inline_mode = true
	};

	embed.fields[1] = (EmbedField) {
		.name = "Memory usage",
		.value = memory_usage,
		.inline_mode = true
	};

	embed.fields[2] = (EmbedField) {
		.name = "Guild count",
		.value = guild_count,
		.inline_mode = true
	};

	embed.color = COLOR;

	send_embed(client, json_get_val(*message, "channel_id").value.string, embed);
	free(embed.fields);
}
