#include <stdlib.h>

#include <png.h>

void png_free(struct PNG png) {
	free(png.png_data);
	free(png.readable_data);
}
