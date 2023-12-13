#include <string.h>

#include <png.h>

unsigned char get_byte_size_of_pixel(const unsigned char color_type) {
	unsigned char pixel_count = 0;

	switch (color_type) {
		case RGB:
			pixel_count = 3;
			break;

		case RGBA:
			pixel_count = 4;
			break;
	}

	return pixel_count;
}

void get_orig_color(const struct PNG png, const unsigned int x, const unsigned int y, unsigned char *orig_color) {
	const unsigned char color_pixels = get_byte_size_of_pixel(png.color_type);
	const size_t start = ((y + 1) + (y * png.width * color_pixels) + (x * color_pixels));
	const unsigned char filter_method = png.data[(y * png.width * color_pixels) + y];

	if (filter_method == 0) {
		memcpy(orig_color, png.data + start, color_pixels);
	} else if (filter_method == 1) {
		if (x == 0) {
			memcpy(orig_color, png.data + start, color_pixels);
		} else if (x == 1) {
			for (unsigned char n = 0; n < color_pixels; ++n) {
				orig_color[n] = png.data[start - color_pixels + n] + png.data[start + n];
			}
		} else {
			memcpy(orig_color, png.data + (y + 1) + (y * png.width * color_pixels), color_pixels);

			for (unsigned int i = 1; i <= x; ++i) {
				const size_t at = ((y + 1) + (y * png.width * color_pixels) + (i * color_pixels));

				for (unsigned char n = 0; n < color_pixels; ++n) {
					orig_color[n] = (orig_color[n] + png.data[at + n]) & 0xFF;
				}
			}
		}
	}
}
