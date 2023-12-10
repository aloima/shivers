#include <stddef.h>
#include <stdbool.h>

#ifndef PNG_H_
	#define PNG_H_

	#define RGB 2
	#define RGBA 6

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

	unsigned char get_byte_size_of_pixel(const unsigned char color_type);

	struct OutputPNG out_png(const struct PNG png);

	void png_free(struct PNG png);
	void opng_free(struct OutputPNG opng);
#endif
