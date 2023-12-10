#include <stdlib.h>

#include <png.h>

void png_free(struct PNG png) {
	free(png.data);
}

void opng_free(struct OutputPNG opng) {
	free(opng.data);
}
