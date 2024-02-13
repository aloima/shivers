#include <stdio.h>
#include <stdbool.h>

#include <shivers.h>
#include <database.h>
#include <discord.h>
#include <utils.h>
#include <json.h>
#include <png.h>

static void execute(const struct Client client, const struct InteractionCommand command) {
	struct Message message = {
		.target_type = TARGET_INTERACTION_COMMAND,
		.target = {
			.interaction_command = command,
		}
	};

	char user_id[20], username[33], discriminator[5], hash[33], avatar_url[104];

	if (command.argument_size == 1) {
		if (strcmp(command.arguments[0].name, "id") == 0) {
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

			strcpy(username, json_get_val(user, "username").value.string);
			strcpy(discriminator, json_get_val(user, "discriminator").value.string);

			jsonresult_t avatar_result = json_get_val(user, "avatar");

			if (avatar_result.exist && avatar_result.type == JSON_STRING) {
				strcpy(hash, avatar_result.value.string);
			}

			get_avatar_url(avatar_url, client.token, user_id, discriminator, hash, true, 256);

			json_free(user, false);
			response_free(&response);
		} else {
			jsonelement_t *user = command.arguments[0].value.user;
			strcpy(user_id, json_get_val(user, "id").value.string);
			strcpy(username, json_get_val(user, "username").value.string);
			strcpy(discriminator, json_get_val(user, "discriminator").value.string);
			jsonresult_t avatar_result = json_get_val(user, "avatar");

			if (avatar_result.exist && avatar_result.type == JSON_STRING) {
				strcpy(hash, avatar_result.value.string);
			}

			get_avatar_url(avatar_url, client.token, user_id, discriminator, hash, true, 256);
		}
	} else {
		strcpy(user_id, json_get_val(command.user, "id").value.string);
		strcpy(username, json_get_val(command.user, "username").value.string);
		strcpy(discriminator, json_get_val(command.user, "discriminator").value.string);
		jsonresult_t avatar_result = json_get_val(command.user, "avatar");

		if (avatar_result.exist && avatar_result.type == JSON_STRING) {
			strcpy(hash, avatar_result.value.string);
		}

		get_avatar_url(avatar_url, client.token, user_id, discriminator, hash, true, 256);
	}

	char xp_key[50], level_key[53], factor_key[42];
	sprintf(xp_key, "%s.levels.%s.xp", command.guild_id, user_id);
	sprintf(level_key, "%s.levels.%s.level", command.guild_id, user_id);
	sprintf(factor_key, "%s.settings.level.factor", command.guild_id);

	const unsigned int xp = (database_has(xp_key) ? database_get(xp_key).number : 0);
	const unsigned int level = (database_has(level_key) ? database_get(level_key).number : 1);
	const unsigned short factor = (database_has(factor_key) ? database_get(factor_key).number : 100);

	char xp_text[16], level_text[16];
	sprintf(level_text, "Level %d", level);
	sprintf(xp_text, "XP %d / %d", xp, (level * factor));

	struct Response response = request((struct RequestConfig) {
		.url = avatar_url,
		.method = "GET"
	});

	struct PNG background_image = {
		.width = 1548,
		.height = 512,
		.color_type = PNG_RGBA_COLOR,
		.is_interlaced = false
	};

	initialize_png(&background_image);

	const unsigned char font_color[3] = {255, 255, 255};
	write_text(&background_image, 518, 184, username, get_fonts().arial, font_color, 18, PNG_TEXT_LEFT);
	write_text(&background_image, 518, 364, level_text, get_fonts().arial, font_color, 16, PNG_TEXT_LEFT);
	write_text(&background_image, 1486, 364, xp_text, get_fonts().arial, font_color, 16, PNG_TEXT_RIGHT);

	unsigned char xp_bar_color[4] = {0, 221, 255, 255};
	const unsigned short xp_bar_width = (968 * ((double) xp / (factor * level)));

	unsigned char empty_bar_color[4] = {163, 163, 163, 255};
	const unsigned short empty_bar_width = (968 - xp_bar_width);

	draw_rect(&background_image, (struct Rectangle) {
		.fill = true,
		.color = xp_bar_color,
		.color_size = 4,
		.height = 64,
		.width = xp_bar_width
	}, 518, 236);

	draw_rect(&background_image, (struct Rectangle) {
		.fill = true,
		.color = empty_bar_color,
		.color_size = 4,
		.height = 64,
		.width = empty_bar_width
	}, (518 + xp_bar_width), 236);

	struct PNG avatar_image = read_png(response.data, response.data_size);

	response_free(&response);

	unsigned char *orig_data_of_avatar;
	get_orig_data(avatar_image, &orig_data_of_avatar);

	struct PNG avatar_image_scaled = scale(avatar_image, orig_data_of_avatar, 384, 384);
	draw_image(&background_image, avatar_image_scaled, 64, 64, true);

	struct OutputPNG opng = out_png(background_image);

	add_file_to_message_payload(&(message.payload), "level.png", (const char *) opng.data, opng.data_size, "image/png");

	send_message(client, message);
	free_message_payload(message.payload);
	png_free(avatar_image);
	free(orig_data_of_avatar);
	png_free(avatar_image_scaled);
	png_free(background_image);
	opng_free(opng);
}

static const struct CommandArgument args[] = {
	(const struct CommandArgument) {
		.name = "member",
		.description = "The mention of a member whose level status that you want to view",
		.type = USER_ARGUMENT,
		.optional = true
	},
	(const struct CommandArgument) {
		.name = "id",
		.description = "The ID of a member whose level status that you want to view",
		.type = STRING_ARGUMENT,
		.optional = true
	}
};

const struct Command level = {
	.execute = execute,
	.name = "level",
	.description = "Displays your level",
	.guild_only = true,
	.args = args,
	.arg_size = sizeof(args) / sizeof(struct CommandArgument)
};
