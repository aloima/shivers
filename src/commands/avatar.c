#include <string.h>
#include <stdlib.h>

#include <sys/resource.h>

#include <shivers.h>
#include <discord.h>
#include <utils.h>

void avatar(Client client, JSONElement **message, Split args) {
	Embed embed;
	memset(&embed, 0, sizeof(Embed));

	char avatar_url[128] = {0};

	if (args.size == 1) {
		char user_id[19] = {0};
		snprintf(user_id, 19, "%s", args.data[0] + 2);

		char path[32] = {0};
		sprintf(path, "/users/%s", user_id);

		Response response = api_request(client.token, path, "GET", NULL);
		JSONElement *user = json_parse(response.data);

		sprintf(avatar_url, "https://cdn.discordapp.com/avatars/%s/%s.png?size=1024", user_id, json_get_val(user, "avatar").string);

		json_free(user);
		response_free(&response);
	} else {
		sprintf(avatar_url, "https://cdn.discordapp.com/avatars/%s/%s.png?size=1024",
			json_get_val(*message, "author.id").string, json_get_val(*message, "author.avatar").string);
	}

	embed.color = COLOR;
	embed.image_url = avatar_url;

	send_embed(client, json_get_val(*message, "d.channel_id").string, embed);
}
