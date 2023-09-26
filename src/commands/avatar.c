#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <shivers.h>
#include <discord.h>
#include <network.h>
#include <json.h>

#define AVATAR_URL "https://cdn.discordapp.com/avatars/%s/%s.%s?size=1024"
#define DEFAULT_AVATAR_URL "https://cdn.discordapp.com/embed/avatars/%d.png?size=1024"

static void execute(struct Client client, jsonelement_t **message, Split args) {
	struct Message reply = {0};
	struct Embed embed = {0};
	const char *channel_id = json_get_val(*message, "channel_id").value.string;

	char avatar_url[101] = {0};

	if (args.size == 1) {
		const char *arg = args.data[0];
		const size_t arg_length = strlen(arg);
		const bool mention_error = (arg_length != 21 || strncmp(arg, "<@", 2) != 0 || arg[20] != '>');

		if (mention_error && (arg_length != 18)) {
			reply.content = INVALID_ARGUMENT;
			send_message(client, channel_id, reply);
			return;
		} else {
			char user_id[19] = {0};

			if (arg_length == 18) {
				snprintf(user_id, 19, "%s", args.data[0]);
			} else {
				snprintf(user_id, 19, "%s", args.data[0] + 2);
			}

			if (!check_snowflake(user_id)) {
				reply.content = INVALID_ARGUMENT;
				send_message(client, channel_id, reply);
				return;
			} else {
				char path[32] = {0};
				sprintf(path, "/users/%s", user_id);

				struct Response response = api_request(client.token, path, "GET", NULL);
				jsonelement_t *user = json_parse(response.data);
				const jsonresult_t avatar = json_get_val(user, "avatar");

				if (avatar.exist && avatar.type != JSON_NULL) {
					const char *avatar_hash = avatar.value.string;
					const char *extension = ((strncmp(avatar_hash, "a_", 2) == 0) ? "gif" : "png");

					sprintf(avatar_url, AVATAR_URL, user_id, avatar_hash, extension);
				} else {
					const char *discriminator = json_get_val(user, "discriminator").value.string;

					sprintf(avatar_url, DEFAULT_AVATAR_URL, atoi(discriminator) % 5);
				}

				json_free(user);
				response_free(&response);
			}
		}
	} else {
		const char *user_id = json_get_val(*message, "author.id").value.string;
		const jsonresult_t avatar = json_get_val(*message, "author.avatar");

		if (avatar.exist && avatar.type != JSON_NULL) {
			const char *avatar_hash = avatar.value.string;
			const char *extension = ((strncmp(avatar_hash, "a_", 2) == 0) ? "gif" : "png");

			sprintf(avatar_url, AVATAR_URL, user_id, avatar_hash, extension);
		} else {
			const char *discriminator = json_get_val(*message, "author.discriminator").value.string;

			sprintf(avatar_url, DEFAULT_AVATAR_URL, atoi(discriminator) % 5);
		}
	}

	embed.image_url = avatar_url;
	embed.color = COLOR;

	add_embed_to_message(embed, &reply);
	send_message(client, channel_id, reply);
	free_message(reply);
}

const struct Command avatar = {
	.execute = execute,
	.name = "avatar",
	.description = "Sends the avatar of the user",
	.args = (struct CommandArgument[]) {
		(struct CommandArgument) {
			.name = "member",
			.description = "The mention or the ID of a member whose avatar that you want to view",
			.examples = (const char *[]) {"840217542400409630", "<@840217542400409630>"},
			.example_size = 2,
			.optional = true
		}
	},
	.arg_size = 1
};
