#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#include <shivers.h>
#include <discord.h>
#include <network.h>
#include <json.h>

static void execute(struct Client client, jsonelement_t *message, const struct Split args) {
	struct Message reply = {0};
	struct Embed embed = {0};
	const char *channel_id = json_get_val(message, "channel_id").value.string;

	char gif_avatar_url[103];
	char png_avatar_url[103];

	char user_id[19], discriminator[5], hash[33];

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

				strcpy(discriminator, json_get_val(user, "discriminator").value.string);
				strcpy(hash, json_get_val(user, "avatar").value.string);

				get_avatar_url(png_avatar_url, client.token, user_id, discriminator, hash, true, 1024);
				get_avatar_url(gif_avatar_url, client.token, user_id, discriminator, hash, false, 1024);

				json_free(user, false);
				response_free(&response);
			}
		}
	} else {
		strcpy(user_id, json_get_val(message, "author.id").value.string);
		strcpy(discriminator, json_get_val(message, "author.discriminator").value.string);
		strcpy(hash, json_get_val(message, "author.avatar").value.string);

		get_avatar_url(gif_avatar_url, client.token, user_id, discriminator, hash, false, 1024);
		get_avatar_url(png_avatar_url, client.token, user_id, discriminator, hash, true, 1024);
	}

	if (strstr(gif_avatar_url, ".gif") != NULL) {
		char description[1536];

		char png_avatar_urls[5][102];
		char gif_avatar_urls[5][104];

		for (unsigned char i = 0; i < 5; ++i) {
			if (i == 2) {
				strcpy(png_avatar_urls[i], png_avatar_url);
			} else {
				const size_t size = pow(2, (8 + i));
				get_avatar_url(png_avatar_urls[i], client.token, user_id, discriminator, hash, true, size);
				get_avatar_url(gif_avatar_urls[i], client.token, user_id, discriminator, hash, false, size);
			}
		}

		sprintf(description, (
			"PNG\\n"
			"[0.25x](%s) **|** [0.5x](%s) **|** [1x](%s) **|** [2x](%s) **|** [4x](%s)\\n\\n"
			"GIF\\n"
			"[0.25x](%s) **|** [0.5x](%s) **|** 1x **|** [2x](%s) **|** [4x](%s)"
		), png_avatar_urls[0], png_avatar_urls[1], png_avatar_urls[2], png_avatar_urls[3], png_avatar_urls[4],
		gif_avatar_urls[0], gif_avatar_urls[1], gif_avatar_urls[3], gif_avatar_urls[4]);

		embed.description = description;
		embed.image_url = gif_avatar_url;
	} else {
		char description[1024];

		char png_avatar_urls[5][102];

		for (unsigned char i = 0; i < 5; ++i) {
			if (i != 2) {
				const size_t size = pow(2, (8 + i));
				get_avatar_url(png_avatar_urls[i], client.token, user_id, discriminator, hash, true, size);
			}
		}

		sprintf(description, (
			"PNG\\n"
			"[0.25x](%s) **|** [0.5x](%s) **|** 1x **|** [2x](%s) **|** [4x](%s)"
		), png_avatar_urls[0], png_avatar_urls[1], png_avatar_urls[3], png_avatar_urls[4]);

		embed.description = description;
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
