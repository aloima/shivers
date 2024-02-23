#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#include <shivers.h>
#include <discord.h>
#include <network.h>
#include <json.h>

static void execute(const struct Client client, const struct InteractionCommand command) {
	struct Embed embed = {0};
	struct Message message = {
		.target_type = TARGET_INTERACTION_COMMAND,
		.target = {
			.interaction_command = command
		}
	};

	char gif_avatar_url[104];
	char png_avatar_url[104];

	char user_id[20], discriminator[5] = {0}, hash[33] = {0};

	if (command.argument_size == 1) {
		if (strsame(command.arguments[0].name, "id")) {
			const char *input = command.arguments[0].value.string;

			if (!check_snowflake(input)) {
				message.payload.content = INVALID_ARGUMENT;
				send_message(client, message);
				return;
			}

			strcpy(user_id, input);

			char path[27] = "/users/";
			strcat(path, user_id);

			struct Response response = api_request(client.token, path, "GET", NULL, NULL);
			jsonelement_t *user = json_parse((const char *) response.data);

			jsonresult_t avatar_result = json_get_val(user, "avatar");

			strcpy(discriminator, json_get_val(user, "discriminator").value.string);

			if (avatar_result.exist && avatar_result.type == JSON_STRING) {
				strcpy(hash, avatar_result.value.string);
			}

			get_avatar_url(png_avatar_url, client.token, user_id, discriminator, hash, true, 1024);
			get_avatar_url(gif_avatar_url, client.token, user_id, discriminator, hash, false, 1024);

			json_free(user, false);
			response_free(&response);
		} else {
			jsonelement_t *user = command.arguments[0].value.user.user_data;
			strcpy(user_id, json_get_val(user, "id").value.string);
			strcpy(discriminator, json_get_val(user, "discriminator").value.string);

			jsonresult_t avatar_result = json_get_val(user, "avatar");

			if (avatar_result.exist && avatar_result.type == JSON_STRING) {
				strcpy(hash, avatar_result.value.string);
			}

			get_avatar_url(png_avatar_url, client.token, user_id, discriminator, hash, true, 1024);
			get_avatar_url(gif_avatar_url, client.token, user_id, discriminator, hash, false, 1024);
		}
	} else if (command.argument_size == 2) {
		message.payload.content = "You cannot specify two arguments, please specify `id` or `member`.";
		send_message(client, message);
		return;
	} else {
		strcpy(user_id, json_get_val(command.user, "id").value.string);
		strcpy(discriminator, json_get_val(command.user, "discriminator").value.string);

		jsonresult_t avatar_result = json_get_val(command.user, "avatar");

		if (avatar_result.exist && avatar_result.type == JSON_STRING) {
			strcpy(hash, avatar_result.value.string);
		}

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
				const unsigned long size = pow(2, (8 + i));
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
				const unsigned long size = pow(2, (8 + i));
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

	add_embed_to_message_payload(embed, &message.payload);
	send_message(client, message);
	free_message_payload(message.payload);
}

static const struct CommandArgument args[] = {
	(const struct CommandArgument) {
		.name = "member",
		.description = "The mention of a member whose avatar that you want to view",
		.type = USER_ARGUMENT,
		.optional = true
	},
	(const struct CommandArgument) {
		.name = "id",
		.description = "The ID of a member whose avatar that you want to view",
		.type = STRING_ARGUMENT,
		.optional = true
	}
};

const struct Command avatar = {
	.execute = execute,
	.name = "avatar",
	.description = "Sends the avatar of the user",
	.guild_only = false,
	.args = args,
	.arg_size = sizeof(args) / sizeof(struct CommandArgument)
};
