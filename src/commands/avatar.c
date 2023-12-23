#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <shivers.h>
#include <discord.h>
#include <network.h>
#include <json.h>

static void execute(struct Client client, jsonelement_t *message, const struct Split args) {
	struct Message reply = {0};
	struct Embed embed = {0};
	const char *channel_id = json_get_val(message, "channel_id").value.string;

	char gif_avatar_url[101];
	char png_avatar_url[101];

	char user_id[19];

	if (args.size == 1) {
		const char *arg = args.data[0].data;
		const size_t arg_length = args.data[0].length;
		const bool mention_error = (arg_length != 21 || strncmp(arg, "<@", 2) != 0 || arg[20] != '>');

		if (mention_error && (arg_length != 18)) {
			reply.content = INVALID_ARGUMENT;
			send_message(client, channel_id, reply);
			return;
		} else {
			if (arg_length == 18) {
				strcpy(user_id, args.data[0].data);
			} else {
				strncpy(user_id, args.data[0].data + 2, 18);
				user_id[18] = 0;
			}

			if (!check_snowflake(user_id)) {
				reply.content = INVALID_ARGUMENT;
				send_message(client, channel_id, reply);
				return;
			} else {
				char path[26] = "/users/";
				strcat(path, user_id);

				struct Response response = api_request(client.token, path, "GET", NULL, NULL);
				jsonelement_t *user = json_parse((const char *) response.data);

				const char *discriminator = json_get_val(user, "discriminator").value.string;
				const char *hash = json_get_val(user, "avatar").value.string;

				get_avatar_url(png_avatar_url, client.token, user_id, discriminator, hash, true);
				get_avatar_url(gif_avatar_url, client.token, user_id, discriminator, hash, false);

				json_free(user, false);
				response_free(&response);
			}
		}
	} else {
		const char *user_id = json_get_val(message, "author.id").value.string;
		const char *discriminator = json_get_val(message, "author.discriminator").value.string;
		const char *hash = json_get_val(message, "author.avatar").value.string;

		get_avatar_url(gif_avatar_url, client.token, user_id, discriminator, hash, false);
		get_avatar_url(png_avatar_url, client.token, user_id, discriminator, hash, true);
	}

	if (strstr(gif_avatar_url, ".gif") != NULL) {
		char description[128];
		sprintf(description, "[Display as PNG](%s)", png_avatar_url);

		embed.description = description;
		embed.image_url = gif_avatar_url;
	} else {
		embed.image_url = png_avatar_url;
	}

	embed.color = COLOR;

	add_embed_to_message(embed, &reply);
	send_message(client, channel_id, reply);
	free_message(reply);
}

static const struct CommandArgument args[] = {
	(const struct CommandArgument) {
		.name = "member",
		.description = "The mention or the ID of a member whose avatar that you want to view",
		.examples = (const char *[]) {"840217542400409630", "<@840217542400409630>"},
		.example_size = 2,
		.optional = true
	}
};

const struct Command avatar = {
	.execute = execute,
	.name = "avatar",
	.description = "Sends the avatar of the user",
	.args = args,
	.arg_size = sizeof(args) / sizeof(struct CommandArgument)
};
