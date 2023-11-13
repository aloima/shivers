#include <stdlib.h>

#include <image.h>

void png_free(struct PNG png) {
	free(png.data);
}

void image_free(struct Image image) {
	free(image.data);
}
