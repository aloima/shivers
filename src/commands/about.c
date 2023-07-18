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

	embed.field_size = 2;
	embed.fields = allocate(NULL, 2, sizeof(EmbedField));
	embed.fields[0] = (EmbedField) {
		.name = "Geliştirici",
		.value = "<@840217542400409630>",
		.inline_mode = true
	};

	embed.fields[1] = (EmbedField) {
		.name = "Bellek kullanımı",
		.value = memory_usage,
		.inline_mode = true
	};

	embed.color = COLOR;


	send_embed(client, json_get_val(*message, "d.channel_id").string, embed);
	free(embed.fields);
}
