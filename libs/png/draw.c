#include <string.h>

#include <png.h>

void set_pixel(struct PNG *png, unsigned int x, unsigned int y, unsigned char *color, unsigned char color_size) {
	if (!png->is_interlaced) {
		const unsigned char png_color_size = get_byte_size_of_pixel(png->color_type);
		const size_t start = ((y + 1) + (y * png->width * png_color_size) + (x * png_color_size));
		const unsigned char filter_method = png->data[(y * png->width * png_color_size) + y];
		unsigned char fix_color[png_color_size];
		memcpy(fix_color, color, color_size);

		if (png->color_type == RGBA && color_size == 3) {
			fix_color[3] = 0xFF;
		}

		if (filter_method == 0) {
			memcpy(png->data + start, color, png_color_size);
		} else if (filter_method == 1) {
			unsigned char next_orig_colors[png_color_size];

			if (x == 0) {
				get_orig_color(*png, 1, y, next_orig_colors);

				const size_t next = ((y + 1) + (y * png->width * png_color_size) + png_color_size);

				for (unsigned char n = 0; n < png_color_size; ++n) {
					png->data[next + n] = (next_orig_colors[n] - fix_color[n]) & 0xFF;
				}
			} else if ((x - 1) == png->width) {
				unsigned char prev_orig_colors[png_color_size];
				get_orig_color(*png, x - 1, y, prev_orig_colors);

				for (unsigned char n = 0; n < png_color_size; ++n) {
					png->data[start + n] = (color[n] - prev_orig_colors[n]) & 0xFF;
				}
			} else {
				unsigned char prev_orig_colors[png_color_size];
				get_orig_color(*png, x - 1, y, prev_orig_colors);
				get_orig_color(*png, x + 1, y, next_orig_colors);

				const size_t next = ((y + 1) + (y * png->width * png_color_size) + ((x + 1) * png_color_size));

				for (unsigned char n = 0; n < png_color_size; ++n) {
					png->data[next + n] = (next_orig_colors[n] - fix_color[n]) & 0xFF;
				}

				for (unsigned char n = 0; n < png_color_size; ++n) {
					png->data[start + n] = (fix_color[n] - prev_orig_colors[n]) & 0xFF;
				}
			}
		}
	}
}

void draw_image(struct PNG *image, const struct PNG data, unsigned int x, unsigned int y) {
	const unsigned char data_color_size = get_byte_size_of_pixel(data.color_type);
	const unsigned char image_color_size = get_byte_size_of_pixel(image->color_type);

	for (unsigned int a = 0; a < data.width; ++a) {
		for (unsigned int b = 0; b < data.height; ++b) {
			unsigned char color[data_color_size];
			get_orig_color(data, a, b, color);

			set_pixel(image, x + a, y + b, color, image_color_size);
		}
	}
}
