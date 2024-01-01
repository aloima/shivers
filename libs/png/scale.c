#include <math.h>
#include <string.h>

#include <png.h>

#define RADIUS 2

static double sinc(double x) {
	if (x == 0.0) {
		return 1.0;
	} else {
		x *= M_PI;
		return (sin(x) / x);
	}
}

// used Lanczos resampling
struct PNG scale(struct PNG png, const unsigned int width, const unsigned int height) {
	const unsigned char color_size = get_byte_size_of_pixel(png.color_type);

	struct PNG new_png = {
		.color_type = png.color_type,
		.height = height,
		.width = width,
		.is_interlaced = png.is_interlaced
	};

	initialize_png(&new_png);

	const double scale_x = ((double) png.width / new_png.width);
	const double scale_y = ((double) png.height / new_png.height);

	for (unsigned int y = 0; y < new_png.height; y++) {
		for (unsigned int x = 0; x < new_png.width; x++) {
			const double source_x = (x * scale_x);
			const double source_y = (y * scale_y);

			const int x_min = (int) (floor(source_x) - RADIUS + 1);
			const int y_min = (int) (floor(source_y) - RADIUS + 1);

			double result[color_size];
			double weight_sum = 0.0;
			memset(result, 0, sizeof(result));

			for (int j = 0; j < 2 * RADIUS; j++) {
				for (int i = 0; i < 2 * RADIUS; i++) {
					int x_index = x_min + i;
					int y_index = y_min + j;

					if (x_index >= 0 && x_index < png.width && y_index >= 0 && y_index < png.height) {
						const double weight_x = sinc(source_x - x_index);
						const double weight_y = sinc(source_y - y_index);
						const double weight = (weight_x * weight_y);

						unsigned char pixels[color_size];
						get_orig_color(png, x_index, y_index, pixels);

						for (unsigned char c = 0; c < color_size; c++) {
							result[c] += (weight * pixels[c]);
						}

						weight_sum += weight;
					}
				}
			}

			if (weight_sum > 0.0) {
				for (int c = 0; c < color_size; c++) {
					result[c] /= weight_sum;
				}

				unsigned char final_pixel[color_size];

				for (int c = 0; c < color_size; c++) {
					final_pixel[c] = (unsigned char) fmin(fmax(result[c], 0.0), 255.0);
				}

				set_pixel(&new_png, x, y, final_pixel, 3);
			}
		}
	}

	return new_png;
}
