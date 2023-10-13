#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <vips/vips.h>

#include <shivers.h>
#include <discord.h>
#include <utils.h>
#include <json.h>

static void execute(struct Client client, jsonelement_t *message, Split args) {
	struct Message reply = {0};

	const size_t total_size = (4 * 1152 * 1152);
	unsigned char *data = allocate(NULL, 0, total_size, sizeof(unsigned char));
	void *png_data;
	size_t png_data_size;

	VipsImage *image = vips_image_new_from_memory(data, total_size, 1152, 1152, 4, VIPS_FORMAT_UCHAR);

	double border_colors[4] = {49, 46, 43, 255};
	vips_draw_rect(image, border_colors, 4, 0, 0, 64, 1152, "fill", true, NULL);
	vips_draw_rect(image, border_colors, 4, 0, 0, 1152, 64, "fill", true, NULL);
	vips_draw_rect(image, border_colors, 4, 1088, 0, 64, 1152, "fill", true, NULL);
	vips_draw_rect(image, border_colors, 4, 0, 1088, 1152, 64, "fill", true, NULL);

	double white_colors[4] = {238, 238, 210, 255};
	double black_colors[4] = {118, 150, 86, 255};

	const char *letters[8] = {"A", "B", "C", "D", "E", "F", "G", "H"};
	const char *numbers[8] = {"1", "2", "3", "4", "5", "6", "7", "8"};

	VipsImage **texts = (VipsImage **) vips_object_local_array(VIPS_OBJECT(image), 16);

	for (short c = 0; c < 8; ++c) {
		char letter_format[64] = {0};
		sprintf(letter_format, "<span background=\"#312E2B\" foreground=\"white\">%s</span>", letters[c]);
		vips_text(&texts[c], letter_format, "font", "sans 16", "dpi", 300, "rgba", true, NULL);

		vips_insert(image, texts[c], &image, 104 + c * 128, 6, NULL);
		vips_insert(image, texts[c], &image, 104 + c * 128, 1092, NULL);

		char number_format[64] = {0};
		sprintf(number_format, "<span background=\"#312E2B\" foreground=\"white\">%s</span>", numbers[7 - c]);
		vips_text(&texts[c + 8], number_format, "font", "sans 16", "dpi", 300, "rgba", true, NULL);

		vips_insert(image, texts[c + 8], &image, 12, 104 + c * 128, NULL);
		vips_insert(image, texts[c + 8], &image, 1098, 104 + c * 128, NULL);

		for (char r = 0; r < 8; ++r) {
			if (c % 2 == 0) {
				if (r % 2 == 0) {
					vips_draw_rect(image, white_colors, 4, 64 + c * 128, 64 + r * 128, 128, 128, "fill", true, NULL);
				} else {
					vips_draw_rect(image, black_colors, 4, 64 + c * 128, 64 + r * 128, 128, 128, "fill", true, NULL);
				}
			} else {
				if (r % 2 == 0) {
					vips_draw_rect(image, black_colors, 4, 64 + c * 128, 64 + r * 128, 128, 128, "fill", true, NULL);
				} else {
					vips_draw_rect(image, white_colors, 4, 64 + c * 128, 64 + r * 128, 128, 128, "fill", true, NULL);
				}
			}
		}
	}

	vips_pngsave_buffer(image, &png_data, &png_data_size, NULL);

	add_file_to_message(&reply, "board.png", png_data, png_data_size, "image/png");
	send_message(client, json_get_val(message, "channel_id").value.string, reply);
	free_message(reply);

	g_object_unref(image);
	g_free(png_data);
	free(data);
}

const struct Command chess = {
	.execute = execute,
	.name = "chess",
	.description = "Makes you play chess",
	.args = NULL,
	.arg_size = 0
};
