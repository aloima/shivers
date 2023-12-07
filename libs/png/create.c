#include <string.h>
#include <stdlib.h>

#include <zlib.h>

#include <png.h>
#include <utils.h>

void initialize_png(struct PNG *png) {
	unsigned char pixel_count;

	switch (png->color_type) {
		case RGB:
			pixel_count = 3;
			break;

		case RGBA:
			pixel_count = 4;
			break;
	}

	png->readable_data_size = (png->width * png->height * pixel_count + png->height);
	png->readable_data = allocate(NULL, 0, png->readable_data_size, sizeof(char));

	z_stream defstream = {
		.zalloc = Z_NULL,
		.zfree = Z_NULL,
		.opaque = Z_NULL,
		.avail_in = png->readable_data_size,
		.next_in = (Bytef *) png->readable_data,
		.avail_out = 0,
		.next_out = (Bytef *) png->png_data
	};

	deflateInit(&defstream, Z_DEFAULT_COMPRESSION);

	png->png_data_size = deflateBound(&defstream, png->readable_data_size);
	png->png_data = allocate(NULL, 0, png->png_data_size, sizeof(unsigned char));
	defstream.avail_out = png->png_data_size;
	defstream.next_out = (Bytef *) png->png_data;

	deflate(&defstream, Z_FINISH);
	deflateEnd(&defstream);

	if (png->png_data_size != defstream.total_out) {
		png->png_data_size = defstream.total_out;
		png->png_data = allocate(png->png_data, -1, defstream.total_out, sizeof(unsigned char));
	}

	png->png_data_size -= 2;
	memcpy(png->zlib_headers, png->png_data, 2);
	memcpy(png->png_data, png->png_data + 2, png->png_data_size);
	png->png_data = allocate(png->png_data, -1, png->png_data_size, sizeof(unsigned char));
}

size_t get_png_size(const struct PNG png) {
	return (8 + 25 + (14 + png.png_data_size) + 12);
}

unsigned char *out_png(const struct PNG png) {
	unsigned char *output = allocate(NULL, 0, get_png_size(png), sizeof(unsigned char));

	/* Signature */
	const unsigned char signature[8] = {
		0x89, 0x50, 0x4E, 0x47,
		0x0D, 0x0A, 0x1A, 0x0A
	};

	memcpy(output, signature, sizeof(signature));
	/* /Signature */


	/* IHDR */
	// 8, 9 and 10 already are zero
	output[11] = 0x0D;
	output[12] = 0x49;
	output[13] = 0x48;
	output[14] = 0x44;
	output[15] = 0x52;

	// Width
	output[16] = ((png.width >> 24) & 0xFF);
	output[17] = ((png.width >> 16) & 0xFF);
	output[18] = ((png.width >> 8) & 0xFF);
	output[19] = (png.width & 0xFF);

	// Height
	output[20] = ((png.height >> 24) & 0xFF);
	output[21] = ((png.height >> 16) & 0xFF);
	output[22] = ((png.height >> 8) & 0xFF);
	output[23] = (png.height & 0xFF);

	// PNG properties
	output[24] = 8;
	output[25] = png.color_type;
	// 26 and 27 are already zero
	output[28] = png.is_interlaced;

	const unsigned long hdr_crc = crc32(0L, (output + 12), 17);
	output[29] = ((hdr_crc >> 24) & 0xFF);
	output[30] = ((hdr_crc >> 16) & 0xFF);
	output[31] = ((hdr_crc >> 8) & 0xFF);
	output[32] = (hdr_crc & 0xFF);
	/* /IHDR */

	/* IDAT */
	const size_t png_real_data_size = (png.png_data_size + 2);
	output[33] = ((png_real_data_size >> 24) & 0xFF);
	output[34] = ((png_real_data_size >> 16) & 0xFF);
	output[35] = ((png_real_data_size >> 8) & 0xFF);
	output[36] = (png_real_data_size & 0xFF);
	memcpy(output + 37, "IDAT", 4);

	output[41] = png.zlib_headers[0];
	output[42] = png.zlib_headers[1];
	memcpy(output + 43, png.png_data, png.png_data_size);

	const unsigned long dat_crc = crc32(0L, (output + 37), (6 + png.png_data_size));
	output[43 + png.png_data_size] = ((dat_crc >> 24) & 0xFF);
	output[44 + png.png_data_size] = ((dat_crc >> 16) & 0xFF);
	output[45 + png.png_data_size] = ((dat_crc >> 8) & 0xFF);
	output[46 + png.png_data_size] = (dat_crc & 0xFF);
	/* /IDAT */

	/* IEND */
	const unsigned char end[12] = {0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4E, 0x44, 0xAE, 0x42, 0x60, 0x82};
	memcpy(output + 47 + png.png_data_size, end, sizeof(end));
	/* /IEND */

	return output;
}
