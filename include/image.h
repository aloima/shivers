#include <stddef.h>
#include <stdbool.h>

#ifndef IMAGE_H_
	#define IMAGE_H_

	struct Rectangle {
		unsigned int width;
		unsigned int height;
		unsigned char *color;
		bool fill;
	};

	struct Image {
		unsigned int width;
		unsigned int height;
		char *data;
	};

	struct PNG {
		unsigned char *data;
		size_t size;
	};

	struct Image create_image(const unsigned int width, const unsigned int height);
	void image_free(struct Image image);

	void set_pixel(struct Image *image, const unsigned int x, const unsigned int y, const unsigned char *color);
	void draw_rect(struct Image *image, const struct Rectangle rectangle, const unsigned int x, const unsigned int y);

	struct Image read_png(const unsigned char *data, const size_t size);
	struct PNG create_png(struct Image *image);
	void png_free(struct PNG png);
#endif
