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

	char user_id[20], username[33], discriminator[5], hash[33] = {0}, avatar_url[104];

	if (command.argument_size == 1) {
		jsonelement_t *user = command.arguments[0].value.user.user_data;
		const jsonresult_t json_is_bot = json_get_val(user, "bot");

		const bool is_in_guild = (command.arguments[0].value.user.member_data != NULL);
		const bool is_bot = (json_is_bot.exist ? json_is_bot.value.boolean : false);

		if (!is_in_guild) {
			message.payload = (struct MessagePayload) {
				.content = "Since the user is not in the server, they cannot have level or experience.",
				.ephemeral = true
			};

			send_message(client, message);
			return;
		}

		if (is_bot) {
			message.payload = (struct MessagePayload) {
				.content = "Since the user is a bot, it cannot have level or experience.",
				.ephemeral = true
			};

			send_message(client, message);
			return;
		}

		jsonresult_t json_id = json_get_val(user, "id");
		memcpy(user_id, json_id.value.string, (json_id.element->size + 1));

		const jsonresult_t json_username = json_get_val(user, "username");
		memcpy(username, json_username.value.string, (json_username.element->size + 1));

		const jsonresult_t json_discriminator = json_get_val(user, "discriminator");
		memcpy(discriminator, json_discriminator.value.string, (json_discriminator.element->size + 1));

		jsonresult_t json_avatar = json_get_val(user, "avatar");

		if (json_avatar.exist && json_avatar.element->type == JSON_STRING) {
			memcpy(hash, json_avatar.value.string, (json_avatar.element->size + 1));
		}

		get_avatar_url(avatar_url, user_id, discriminator, hash, true, 256);
	} else {
		const jsonresult_t json_user_id = json_get_val(command.user, "id");
		memcpy(user_id, json_user_id.value.string, (json_user_id.element->size + 1));

		const jsonresult_t json_username = json_get_val(command.user, "username");
		memcpy(username, json_username.value.string, (json_username.element->size + 1));

		const jsonresult_t json_discriminator = json_get_val(command.user, "discriminator");
		memcpy(discriminator, json_discriminator.value.string, (json_discriminator.element->size + 1));

		jsonresult_t json_avatar = json_get_val(command.user, "avatar");

		if (json_avatar.exist && json_avatar.element->type == JSON_STRING) {
			memcpy(hash, json_avatar.value.string, (json_avatar.element->size + 1));
		}

		get_avatar_url(avatar_url, user_id, discriminator, hash, true, 256);
	}

	char xp_key[50], level_key[53], factor_key[42], levels_key[27];
	sprintf(xp_key, "%s.levels.%s.xp", command.guild_id, user_id);
	sprintf(level_key, "%s.levels.%s.level", command.guild_id, user_id);
	sprintf(factor_key, "%s.settings.level.factor", command.guild_id);
	sprintf(levels_key, "%s.levels", command.guild_id);

	const jsonresult_t factor_data = database_get(factor_key);
	const unsigned short factor = (factor_data.exist ? factor_data.value.number : 100);

	const jsonresult_t levels = database_get(levels_key);
	const unsigned int sorted_length = (levels.exist ? levels.element->size : 0);
	struct Sort sorted[sorted_length];

	for (unsigned int i = 0; i < sorted_length; ++i) {
		sorted[i].value = ((jsonelement_t **) levels.element->value)[i];

		jsonelement_t *value = sorted[i].value;
		const jsonresult_t json_level = json_get_val(value, "level");

		const unsigned int level = (json_level.exist ? json_level.value.number : 0);
		sorted[i].number = (json_get_val(value, "xp").value.number + (level * (level + 1) * factor / 2.0));
	}

	if (sorted_length != 0) {
		sort(sorted, sorted_length);
	}

	const jsonresult_t xp_data = database_get(xp_key);
	const jsonresult_t level_data = database_get(level_key);

	const unsigned int xp = (xp_data.exist ? xp_data.value.number : 0);
	const unsigned int level = (level_data.exist ? level_data.value.number : 1);

	char xp_text[24], level_text[16], rank[12];
	sprintf(level_text, "Level %d", level);
	sprintf(xp_text, "XP %d / %d", xp, (level * factor));

	rank[0] = 0;

	for (unsigned int i = 0; i < sorted_length; ++i) {
		if (strsame(user_id, ((jsonelement_t *) sorted[i].value)->key)) {
			sprintf(rank, "#%u", (i + 1));
			break;
		}
	}

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
	write_text(&background_image, 518, 184, username, get_fonts().josefinsans, font_color, 18, PNG_TEXT_LEFT);
	write_text(&background_image, 518, 364, level_text, get_fonts().josefinsans, font_color, 16, PNG_TEXT_LEFT);
	write_text(&background_image, 1486, 364, xp_text, get_fonts().josefinsans, font_color, 16, PNG_TEXT_RIGHT);

	if (rank[0] != 0) {
		write_text(&background_image, 1486, 184, rank, get_fonts().josefinsans, font_color, 16, PNG_TEXT_RIGHT);
	}

	unsigned char xp_bar_color[4] = {0, 221, 255, 255};
	const unsigned short xp_bar_width = (968 * ((double) xp / (factor * level)));

	unsigned char empty_bar_color[4] = {163, 163, 163, 255};

	draw_rect(&background_image, (struct Rectangle) {
		.fill = true,
		.color = empty_bar_color,
		.color_size = 4,
		.height = 64,
		.width = 968,
		.border_radius = 15
	}, 518, 236);

	draw_rect(&background_image, (struct Rectangle) {
		.fill = true,
		.color = xp_bar_color,
		.color_size = 4,
		.height = 64,
		.width = xp_bar_width,
		.border_radius = 15
	}, 518, 236);

	struct PNG avatar_image = read_png(response.data, response.data_size);
	palette_to_rgb(&avatar_image);

	response_free(response);

	unsigned char *orig_data_of_avatar;
	get_orig_data(avatar_image, &orig_data_of_avatar);
	png_free(avatar_image);

	struct PNG avatar_image_scaled = scale(avatar_image, orig_data_of_avatar, 384, 384);
	draw_image(&background_image, avatar_image_scaled, 64, 64, true);
	png_free(avatar_image_scaled);
	free(orig_data_of_avatar);

	struct OutputPNG opng = out_png(background_image);
	png_free(background_image);

	add_file_to_message_payload(&(message.payload), "level.png", (const char *) opng.data, opng.data_size, "image/png");
	opng_free(opng);

	send_message(client, message);
	free_message_payload(message.payload);
}

static const struct CommandArgument args[] = {
	(const struct CommandArgument) {
		.name = "member",
		.description = "The mention of a member whose level status that you want to view",
		.type = USER_ARGUMENT,
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
