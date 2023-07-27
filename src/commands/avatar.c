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
		const char *arg = args.data[0];
		const size_t arg_length = strlen(arg);
		bool mention_error = (strncmp(arg, "<@", 2) != 0 || arg[20] != '>');

		if ((arg_length != 21 || mention_error) && (arg_length != 18)) {
			send_content(client, json_get_val(*message, "channel_id").value.string, "Invalid argument, please use `help` command.");
			return;
		} else {
			char user_id[19] = {0};

			if (arg_length == 18) {
				snprintf(user_id, 19, "%s", args.data[0]);
			} else {
				snprintf(user_id, 19, "%s", args.data[0] + 2);
			}

			if (!check_snowflake(user_id)) {
				send_content(client, json_get_val(*message, "channel_id").value.string, "Invalid argument, please use `help` command.");
				return;
			} else {
				char path[32] = {0};
				sprintf(path, "/users/%s", user_id);

				Response response = api_request(client.token, path, "GET", NULL);
				JSONElement *user = json_parse(response.data);
				char *avatar_hash = json_get_val(user, "avatar").value.string;
				bool animated = (strncmp(avatar_hash, "a_", 2) == 0);

				sprintf(avatar_url, "https://cdn.discordapp.com/avatars/%s/%s.%s?size=1024", user_id, avatar_hash, animated ? "gif" : "png");

				json_free(user);
				response_free(&response);
			}
		}
	} else {
		char *avatar_hash = json_get_val(*message, "author.avatar").value.string;
		bool animated = (strncmp(avatar_hash, "a_", 2) == 0);

		sprintf(avatar_url, "https://cdn.discordapp.com/avatars/%s/%s.%s?size=1024",
			json_get_val(*message, "author.id").value.string, avatar_hash, animated ? "gif" : "png");
	}

	embed.color = COLOR;
	embed.image_url = avatar_url;

	send_embed(client, json_get_val(*message, "channel_id").value.string, embed);
}
