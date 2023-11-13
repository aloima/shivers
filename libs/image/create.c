#include <string.h>
#include <stdlib.h>

#include <zlib.h>

#include <image.h>
#include <utils.h>

struct Image create_image(const unsigned int width, const unsigned int height) {
	struct Image image = {
		.width = width,
		.height = height,
		.data = allocate(NULL, 0, width * height * 3 + height, sizeof(char))
	};

	return image;
}

struct PNG create_png(struct Image *image) {
	const unsigned char signature[] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
	const unsigned char header[] = {0x00, 0x00, 0x00, 0x0D, 0x49, 0x48, 0x44, 0x52};
	const unsigned char size_header[] = {
		(image->width >> 24) & 0xFF,
		(image->width >> 16) & 0xFF,
		(image->width >> 8) & 0xFF,
		image->width & 0xFF,
		(image->height >> 24) & 0xFF,
		(image->height >> 16) & 0xFF,
		(image->height >> 8) & 0xFF,
		image->height & 0xFF
	};

	unsigned char png_settings[] = {0x08, 0x02, 0x0, 0x0, 0x0, 0x90, 0x77, 0x53, 0xDE};

	char temp[17];
	memcpy(temp, header + 4, 4);
	memcpy(temp + 4, size_header, 8);
	memcpy(temp + 12, png_settings, 5);

	unsigned int ihdr_crc = crc32(0L, (const Bytef *) temp, 17);
	png_settings[5] = (ihdr_crc >> 24) & 0xFF;
	png_settings[6] = (ihdr_crc >> 16) & 0xFF;
	png_settings[7] = (ihdr_crc >> 8) & 0xFF;
	png_settings[8] = ihdr_crc & 0xFF;

	char *output = NULL;
	const unsigned int data_length = (image->width * image->height * 3 + image->height);

	z_stream defstream = {
		.zalloc = Z_NULL,
		.zfree = Z_NULL,
		.opaque = Z_NULL,
		.avail_in = data_length,
		.next_in = (Bytef *) image->data,
		.avail_out = 0,
		.next_out = (Bytef *) output
	};

	deflateInit(&defstream, Z_DEFAULT_COMPRESSION);

	const unsigned long output_size = deflateBound(&defstream, data_length);
	output = allocate(NULL, 0, output_size, sizeof(char));
	defstream.avail_out = output_size;
	defstream.next_out = (Bytef *) output;

	deflate(&defstream, Z_FINISH);
	deflateEnd(&defstream);

	const size_t png_data_size = output_size - 2;
	char *data = allocate(NULL, 0, 14 + png_data_size, sizeof(char));
	data[0] = (png_data_size >> 24) & 0xFF;
	data[1] = (png_data_size >> 16) & 0xFF;
	data[2] = (png_data_size >> 8) & 0xFF;
	data[3] = png_data_size & 0xFF;
	memcpy(data + 4, "IDAT", 4);
	data[8] = 0x08;
	data[9] = 0xD7;
	memcpy(data + 10, output + 2, png_data_size);

	free(output);

	unsigned int idat_crc = crc32(0L, (const Bytef *) (data + 4), 6 + png_data_size);
	data[10 + png_data_size] = (idat_crc >> 24) & 0xFF;
	data[11 + png_data_size] = (idat_crc >> 16) & 0xFF;
	data[12 + png_data_size] = (idat_crc >> 8) & 0xFF;
	data[13 + png_data_size] = idat_crc & 0xFF;

	const unsigned char end[] = {
		0x00, 0x00, 0x00, 0x00,
		0x49, 0x45, 0x4E, 0x44,
		0xAE, 0x42, 0x60, 0x82
	};

	struct PNG png = {
		.size = 57 + output_size
	};

	png.data = allocate(NULL, -1, png.size, sizeof(char));
	memcpy(png.data, signature, 8);
	memcpy(png.data + 8, header, 8);
	memcpy(png.data + 16, size_header, 8);
	memcpy(png.data + 24, png_settings, 9);
	memcpy(png.data + 33, data, 12 + output_size);
	memcpy(png.data + 45 + output_size, end, 12);

	free(data);
	return png;
}
