#include <string.h>
#include <stdlib.h>

#include <png.h>

void set_pixel(struct PNG *png, const unsigned int x, const unsigned int y, const unsigned char *color, const unsigned char color_size) {
	if (!png->is_interlaced) {
		const unsigned char png_color_size = get_byte_size_of_pixel(png->color_type);
		const unsigned long start = ((y + 1) + (y * png->width * png_color_size) + (x * png_color_size));
		const unsigned char filter_method = png->data[(y * png->width * png_color_size) + y];
		unsigned char fix_color[png_color_size];
		memcpy(fix_color, color, color_size);

		if (png->color_type == RGBA_COLOR && color_size == 3) {
			fix_color[3] = 0xFF;
		}

		if (filter_method == 0) {
			memcpy(png->data + start, color, png_color_size);
		} // TODO: other filter methods for png parameter will be added.
	}
}

void draw_image(struct PNG *image, const struct PNG data, const unsigned int x, const unsigned int y, const bool as_circle) {
	const unsigned char data_color_size = get_byte_size_of_pixel(data.color_type);
	const unsigned char image_color_size = get_byte_size_of_pixel(image->color_type);

	unsigned char *unfiltered_data, color[data_color_size];
	get_orig_data(data, &unfiltered_data);

	if (as_circle && (data.width == data.height)) {
		const unsigned int radius = (data.width / 2);
		const unsigned int rr = (radius * radius);

		for (unsigned int a = 0; a < data.width; ++a) {
			const unsigned int aa = ((a - radius) * (a - radius));

			for (unsigned int b = 0; b < data.height; ++b) {
				if ((aa + ((b - radius) * (b - radius))) <= rr) {
					get_pixel_from_data(data, unfiltered_data, a, b, color);

					set_pixel(image, x + a, y + b, color, image_color_size);
				}
			}
		}
	} else {
		for (unsigned int a = 0; a < data.width; ++a) {
			for (unsigned int b = 0; b < data.height; ++b) {
				get_pixel_from_data(data, unfiltered_data, a, b, color);

				set_pixel(image, x + a, y + b, color, image_color_size);
			}
		}
	}

	free(unfiltered_data);
}

void draw_circle(struct PNG *png, const struct Circle circle, const unsigned int x, const unsigned int y) {
	const unsigned int rr = (circle.radius * circle.radius);
	const unsigned int a_limit = (x + circle.radius);
	const unsigned int b_limit = (y + circle.radius);

	if (circle.fill) {
		for (unsigned int a = 0; a < a_limit; ++a) {
			const unsigned int aa = ((a - circle.radius) * (a - circle.radius));

			for (unsigned int b = 0; b < b_limit; ++b) {
				if ((aa + ((b - circle.radius) * (b - circle.radius))) <= rr) {
					set_pixel(png, a + x - circle.radius, b + y - circle.radius, circle.color, circle.color_size);
				}
			}
		}
	}
}

void draw_rect(struct PNG *png, const struct Rectangle rectangle, const unsigned int x, const unsigned int y) {
	if (rectangle.fill) {
		if (rectangle.width == (rectangle.border_radius * 2) && rectangle.height == rectangle.width) {
			const unsigned int rr = (rectangle.border_radius * rectangle.border_radius);
			const unsigned int a_limit = (x + (rectangle.border_radius * 2));
			const unsigned int b_limit = (y + (rectangle.border_radius * 2));

			for (unsigned int a = 0; a < a_limit; ++a) {
				const unsigned int aa = ((a - rectangle.border_radius) * (a - rectangle.border_radius));

				for (unsigned int b = 0; b < b_limit; ++b) {
					const unsigned int bb = ((b - rectangle.border_radius) * (b - rectangle.border_radius));

					if ((aa + bb) <= rr) {
						set_pixel(png, a + x, b + y, rectangle.color, rectangle.color_size);
					}
				}
			}
		}
	}
}
