#include <stdbool.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#ifndef PNG_H_
	#define PNG_H_

	#define PNG_RGB_COLOR 2
	#define PNG_PALETTE_COLOR 3
	#define PNG_RGBA_COLOR 6

	#define PNG_TEXT_LEFT 0
	#define PNG_TEXT_CENTER 1
	#define PNG_TEXT_RIGHT 2

	struct Circle {
		unsigned int radius;
		unsigned char *color, color_size;
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

	struct Fonts {
		FT_Face arial;
		FT_Face quicksand;
	};

	struct PNG {
		unsigned int width, height;
		unsigned char color_type, *data, *palette;
		bool is_interlaced;
		unsigned short palette_size;
		unsigned long data_size;
	};

	struct OutputPNG {
		unsigned char *data;
		unsigned long data_size;
	};

	void initialize_png(struct PNG *png);
	struct PNG read_png(unsigned char *input, const unsigned long input_size);

	struct PNG scale(const struct PNG png, const unsigned char *orig_data, const unsigned int width, const unsigned int height);

	void initialize_fonts();
	struct Fonts get_fonts();
	void free_fonts();
	void write_text(struct PNG *png, unsigned int x, unsigned int y, const char *text, FT_Face font, const unsigned char *color, const unsigned char size, const unsigned char text_alignment);

	void draw_image(struct PNG *image, const struct PNG data, const unsigned int x, const unsigned int y, const bool as_circle);
	void draw_circle(struct PNG *png, const struct Circle circle, const unsigned int x, const unsigned int y);
	void draw_rect(struct PNG *png, const struct Rectangle rectangle, const unsigned int x, const unsigned int y);
	void set_pixel(struct PNG *png, const unsigned int x, const unsigned int y, const unsigned char *color, const unsigned char color_size);

	void palette_to_rgb(struct PNG *png);
	unsigned char paeth_predictor(const unsigned char a, const unsigned char b, const unsigned char c);
	unsigned char get_byte_size_of_pixel(const unsigned char color_type);

	void get_orig_data(const struct PNG png, unsigned char **data);
	unsigned long get_orig_size(const struct PNG png);
	void get_pixel_from_data(const struct PNG png, const unsigned char *data, const unsigned int x, const unsigned int y, unsigned char *pixel);

	struct OutputPNG out_png(const struct PNG png);

	void png_free(struct PNG png);
	void opng_free(struct OutputPNG opng);
#endif
