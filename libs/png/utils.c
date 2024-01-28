#include <string.h>
#include <stdlib.h>
#include <math.h>

#include <png.h>
#include <utils.h>

unsigned char get_byte_size_of_pixel(const unsigned char color_type) {
	unsigned char pixel_count = 0;

	switch (color_type) {
		case RGB_COLOR:
			pixel_count = 3;
			break;

		case RGBA_COLOR:
			pixel_count = 4;
			break;
	}

	return pixel_count;
}

unsigned char paeth_predictor(const unsigned char a, const unsigned char b, const unsigned char c) {
	const int p = (a + b - c);
	const unsigned int pa = abs(p - a);
	const unsigned int pb = abs(p - b);
	const unsigned int pc = abs(p - c);

	if (pa <= pb && pa <= pc) {
		return a;
	} else if (pb <= pc) {
		return b;
	} else {
		return c;
	}
}

unsigned long get_orig_size(const struct PNG png) {
	const unsigned char color_size = get_byte_size_of_pixel(png.color_type);

	return (png.width * png.height * color_size);
}

void get_orig_data(const struct PNG png, unsigned char **data) {
	const unsigned char color_pixels = get_byte_size_of_pixel(png.color_type);

	*data = allocate(NULL, -1, get_orig_size(png), sizeof(unsigned char));

	for (unsigned int y = 0; y < png.height; ++y) {
		const unsigned char filter_code = png.data[(y * png.width * color_pixels) + y];

		if (filter_code == 0) {
			for (unsigned int x = 0; x < png.width; ++x) {
				const unsigned int pos = ((y + 1) + ((y * png.width) + x) * color_pixels);
				memcpy(*data + ((x + (y * png.width)) * color_pixels), png.data + pos, color_pixels);
			}
		} else if (filter_code == 1) {
			memcpy(*data + (y * png.width * color_pixels), png.data + ((y + 1) + (y * png.width * color_pixels)), color_pixels);

			for (unsigned int x = 1; x < png.width; ++x) {
				const unsigned int at_a = (((y * png.width) + (x - 1)) * color_pixels);
				const unsigned int at_x = ((y + 1) + ((y * png.width) + x) * color_pixels);

				for (unsigned char c = 0; c < color_pixels; ++c) {
					(*data)[((x + (y * png.width)) * color_pixels) + c] = (png.data[at_x + c] + (*data)[at_a + c]);
				}
			}
		} else if (filter_code == 2) {
			if (y == 0) {
				for (unsigned int x = 0; x < png.width; ++x) {
					const unsigned int at_x = (1 + (x * color_pixels));

					for (unsigned char c = 0; c < color_pixels; ++c) {
						(*data)[(x * color_pixels) + c] = (png.data[at_x + c]);
					}
				}
			} else {
				for (unsigned int x = 0; x < png.width; ++x) {
					const unsigned int at_b = ((((y - 1) * png.width) + x) * color_pixels);
					const unsigned int at_x = ((y + 1) + ((y * png.width) + x) * color_pixels);

					for (unsigned char c = 0; c < color_pixels; ++c) {
						(*data)[((x + (y * png.width)) * color_pixels) + c] = (png.data[at_x + c] + (*data)[at_b + c]);
					}
				}
			}
		} else if (filter_code == 3) {
			if (y == 0) {
				memcpy(*data, png.data + 1, color_pixels);

				for (unsigned int x = 1; x < png.width; ++x) {
					const unsigned int at_a = ((x - 1) * color_pixels);
					const unsigned int at_x = (1 + (x * color_pixels));

					for (unsigned char c = 0; c < color_pixels; ++c) {
						(*data)[((x + (y * png.width)) * color_pixels) + c] = (png.data[at_x + c] + floor(((*data)[at_a + c] / 2.0)));
					}
				}
			} else {
				{
					const unsigned int at_b = (((y - 1) * png.width) * color_pixels);
					const unsigned int at_x = ((y + 1) + (y * png.width * color_pixels));

					for (unsigned char c = 0; c < color_pixels; ++c) {
						(*data)[((y * png.width) * color_pixels) + c] = (png.data[at_x + c] + floor(((*data)[at_b + c] / 2.0)));
					}
				}

				for (unsigned int x = 1; x < png.width; ++x) {
					const unsigned int at_b = ((((y - 1) * png.width) + x) * color_pixels);
					const unsigned int at_a = (((y * png.width) + (x - 1)) * color_pixels);
					const unsigned int at_x = ((y + 1) + ((y * png.width) + x) * color_pixels);

					for (unsigned char c = 0; c < color_pixels; ++c) {
						(*data)[((x + (y * png.width)) * color_pixels) + c] = (png.data[at_x + c] + floor((((*data)[at_a + c] + (*data)[at_b + c]) / 2.0)));
					}
				}
			}
		} else if (filter_code == 4) {
			if (y == 0) {
				memcpy(*data, png.data + 1, color_pixels);

				for (unsigned int x = 1; x < png.width; ++x) {
					const unsigned int at_a = ((x - 1) * color_pixels);
					const unsigned int at_x = (1 + (x * color_pixels));

					for (unsigned char c = 0; c < color_pixels; ++c) {
						const unsigned char value = paeth_predictor((*data)[at_a + c], 0, 0);
						(*data)[((x + (y * png.width)) * color_pixels) + c] = (png.data[at_x + c] + value);
					}
				}
			} else {
				{
					const unsigned int at_b = (((y - 1) * png.width) * color_pixels);
					const unsigned int at_x = ((y + 1) + (y * png.width * color_pixels));

					for (unsigned char c = 0; c < color_pixels; ++c) {
						(*data)[((y * png.width) * color_pixels) + c] = (png.data[at_x + c] + paeth_predictor(0, (*data)[at_b + c], 0));
					}
				}

				for (unsigned int x = 1; x < png.width; ++x) {
					const unsigned int at_c = ((((y - 1) * png.width) + (x - 1)) * color_pixels);
					const unsigned int at_b = ((((y - 1) * png.width) + x) * color_pixels);
					const unsigned int at_a = (((y * png.width) + (x - 1)) * color_pixels);
					const unsigned int at_x = ((y + 1) + ((y * png.width) + x) * color_pixels);

					for (unsigned char c = 0; c < color_pixels; ++c) {
						const unsigned char value = paeth_predictor((*data)[at_a + c], (*data)[at_b + c], (*data)[at_c + c]);
						(*data)[((x + (y * png.width)) * color_pixels) + c] = (png.data[at_x + c] + value);
					}
				}
			}
		}
	}
}

void get_pixel_from_data(const struct PNG png, const unsigned char *data, const unsigned int x, const unsigned int y, unsigned char *pixel) {
	const unsigned char color_pixels = get_byte_size_of_pixel(png.color_type);
	memcpy(pixel, data + ((x + (y * png.width)) * color_pixels), color_pixels);
}
