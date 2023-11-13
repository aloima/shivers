#include <image.h>

void draw_rect(struct Image *image, const struct Rectangle rectangle, const unsigned int x, const unsigned int y) {
	if (rectangle.fill) {
		for (int r = 0; r < rectangle.width; ++r) {
			for (int c = 0; c < rectangle.width; ++c) {
				set_pixel(image, x + r, y + c, rectangle.color);
			}
		}
	} else {
		for (int r = 0; r < rectangle.width; ++r) {
			set_pixel(image, x + r, y, rectangle.color);
			set_pixel(image, x + r, y + rectangle.height - 1, rectangle.color);
		}

		for (int c = 0; c < rectangle.width; ++c) {
			set_pixel(image, x, y + c, rectangle.color);
			set_pixel(image, x + rectangle.width - 1, y + c, rectangle.color);
		}
	}
}

void set_pixel(struct Image *image, const unsigned int x, const unsigned int y, const unsigned char *color) {
	const unsigned int index = (x + (y * image->width));
	image->data[index * 3 + y + 1] = color[0];
	image->data[index * 3 + y + 2] = color[1];
	image->data[index * 3 + y + 3] = color[2];
}
