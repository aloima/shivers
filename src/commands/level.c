#include <stdio.h>
#include <stdbool.h>

#include <shivers.h>
#include <database.h>
#include <discord.h>
#include <utils.h>
#include <json.h>
#include <png.h>

static void execute(struct Client client, jsonelement_t *message, const struct Split args) {
	struct Message reply = {0};

	const char *user_id = json_get_val(message, "author.id").value.string;
	const char *user_discriminator = json_get_val(message, "author.discriminator").value.string;
	const char *user_avatar = json_get_val(message, "author.avatar").value.string;

	char xp_key[22], level_key[25];
	sprintf(xp_key, "%s.xp", user_id);
	sprintf(level_key, "%s.level", user_id);

	char xp[12], level[12];
	sprintf(xp, "%.0f", database_has(xp_key) ? database_get(xp_key).number : 0.0);
	sprintf(level, "%.0f", database_has(level_key) ? database_get(level_key).number : 0.0);

	char avatar_url[101];
	get_avatar_url(avatar_url, client.token, user_id, user_discriminator, user_avatar, true);

	struct Response response = request((struct RequestConfig) {
		.url = avatar_url,
		.method = "GET"
	});

	struct PNG background_image = {
		.width = 1548,
		.height = 512,
		.color_type = RGB,
		.is_interlaced = false
	};

	initialize_png(&background_image);

	struct PNG avatar_image = read_png(response.data, response.data_size);

	response_free(&response);

	struct PNG avatar_image_scaled = scale(avatar_image, 384, 384);
	draw_image(&background_image, avatar_image_scaled, 64, 64, true);

	struct OutputPNG opng = out_png(background_image);
	add_file_to_message(&reply, "level.png", (const char *) opng.data, opng.data_size, "image/png");

	send_message(client, json_get_val(message, "channel_id").value.string, reply);
	free_message(reply);
	png_free(avatar_image);
	png_free(avatar_image_scaled);
	png_free(background_image);
	opng_free(opng);
}

const struct Command level = {
	.execute = execute,
	.name = "level",
	.description = "Display your level"
};
