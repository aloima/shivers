#include <string.h>

#include <shivers.h>
#include <discord.h>
#include <network.h>
#include <json.h>

#define AVATAR_URL "https://cdn.discordapp.com/avatars/%s/%s.%s?size=1024"

static void execute(Client client, jsonelement_t **message, Split args) {
	Embed embed = {0};

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
				jsonelement_t *user = json_parse(response.data);
				char *avatar_hash = json_get_val(user, "avatar").value.string;
				char *extension = ((strncmp(avatar_hash, "a_", 2) == 0) ? "gif" : "png");

				sprintf(avatar_url, AVATAR_URL,  user_id, avatar_hash, extension);

				json_free(user);
				response_free(&response);
			}
		}
	} else {
		char *user_id = json_get_val(*message, "author.id").value.string;
		char *avatar_hash = json_get_val(*message, "author.avatar").value.string;
		char *extension = ((strncmp(avatar_hash, "a_", 2) == 0) ? "gif" : "png");

		sprintf(avatar_url, AVATAR_URL, user_id, avatar_hash, extension);
	}

	embed.image_url = avatar_url;
	embed.color = COLOR;

	send_embed(client, json_get_val(*message, "channel_id").value.string, embed);
}

struct Command avatar = {
	.execute = execute,
	.name = "avatar",
	.description = "Sends the avatar of the user",
	.args = NULL
};
