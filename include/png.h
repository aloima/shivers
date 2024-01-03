#include <stddef.h>
#include <stdbool.h>

#ifndef PNG_H_
	#define PNG_H_

	#define RGB 2
	#define RGBA 6

	struct Circle {
		unsigned int radius;
		unsigned char *color;
		unsigned char color_size;
		bool fill;
	};

	struct Rectangle {
		unsigned int border_radius;
		unsigned int width;
		unsigned int height;
		unsigned char *color;
		unsigned char color_size;
		bool fill;
	};

	struct PNG {
		unsigned int width;
		unsigned int height;
		unsigned char color_type;
		bool is_interlaced;
		unsigned char *data;
		size_t data_size;
		unsigned char zlib_headers[2];
	};

	struct OutputPNG {
		unsigned char *data;
		size_t data_size;
	};

	void initialize_png(struct PNG *png);
	struct PNG read_png(unsigned char *input, const size_t input_size);

	struct PNG scale(const struct PNG png, const unsigned int width, const unsigned int height);

	void draw_image(struct PNG *image, const struct PNG data, const unsigned int x, const unsigned int y, const bool as_circle);
	void draw_circle(struct PNG *png, const struct Circle circle, const unsigned int x, const unsigned int y);
	void draw_rect(struct PNG *png, const struct Rectangle rectangle, const unsigned int x, const unsigned int y);
	void set_pixel(struct PNG *png, const unsigned int x, const unsigned int y, const unsigned char *color, const unsigned char color_size);

	unsigned char get_byte_size_of_pixel(const unsigned char color_type);
	void get_orig_color(const struct PNG png, const unsigned int x, const unsigned int y, unsigned char *orig_color);
	void get_pixel_data(const struct PNG png, const unsigned int x, const unsigned int y, unsigned char *data);

	struct OutputPNG out_png(const struct PNG png);

	void png_free(struct PNG png);
	void opng_free(struct OutputPNG opng);
#endif
