#include <string.h>
#include <stdlib.h>

#include <shivers.h>
#include <discord.h>
#include <utils.h>

void about(Client client, JSONElement **message) {
	Embed embed;
	memset(&embed, 0, sizeof(Embed));

	embed.description = allocate(NULL, 6, sizeof(char));
	embed.color = COLOR;
	strcpy(embed.description, "selam");
	send_embed(client, json_get_val(*message, "d.channel_id").string, embed);
	free(embed.description);
}
