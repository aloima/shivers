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

	const char *user_id = json_get_val(command.user, "id").value.string;
	const char *user_discriminator = json_get_val(command.user, "discriminator").value.string;
	const char *user_avatar = json_get_val(command.user, "avatar").value.string;

	char xp_key[22], level_key[25];
	sprintf(xp_key, "%s.levels.%s.xp", command.guild_id, user_id);
	sprintf(level_key, "%s.levels.%s.level", command.guild_id, user_id);

	char xp[12], level[12], *username = json_get_val(command.user, "username").value.string;
	sprintf(xp, "%.0f", database_has(xp_key) ? database_get(xp_key).number : 0.0);
	sprintf(level, "%.0f", database_has(level_key) ? database_get(level_key).number : 0.0);

	char avatar_url[101];
	get_avatar_url(avatar_url, client.token, user_id, user_discriminator, user_avatar, true, 256);

	struct Response response = request((struct RequestConfig) {
		.url = avatar_url,
		.method = "GET"
	});

	struct PNG background_image = {
		.width = 1548,
		.height = 512,
		.color_type = RGBA_COLOR,
		.is_interlaced = false
	};

	initialize_png(&background_image);

	const unsigned char font_color[3] = {255, 255, 255};
	write_text(&background_image, 518, 184, username, get_fonts().arial, font_color, 18);

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

const struct Command level = {
	.execute = execute,
	.name = "level",
	.description = "Displays your level"
};
