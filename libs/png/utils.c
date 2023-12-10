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
