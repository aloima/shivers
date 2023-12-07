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
		unsigned char *png_data;
		unsigned char *readable_data;
		unsigned char zlib_headers[2];
		size_t png_data_size;
		size_t readable_data_size;
	};

	void initialize_png(struct PNG *png);
	unsigned char *out_png(const struct PNG png);
	size_t get_png_size(const struct PNG png);
	void png_free(struct PNG png);
#endif
