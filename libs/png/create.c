#include <string.h>
#include <stdlib.h>

#include <zlib.h>

#include <png.h>
#include <network.h>
#include <utils.h>

void initialize_png(struct PNG *png) {
	png->data_size = (png->width * png->height * get_byte_size_of_pixel(png->color_type) + png->height);
	png->data = allocate(NULL, 0, png->data_size, sizeof(char));
}

struct PNG read_png(unsigned char *input, const size_t input_size) {
	struct PNG png = {0};

	bool taken_headers = false;
	unsigned char *data = NULL;
	size_t data_size = 0;

	for (size_t i = 0; i < input_size; ++i) {
		const unsigned char ch = input[i];

		if (ch == 'I') {
			i += 1;
			const char *chunk_name = (const char *) (input + i);

			if (strncmp(chunk_name, "HDR", 3) == 0) {
				i += 3;
				png.width = combine_bytes(input + i, 4);

				i += 4;
				png.height = combine_bytes(input + i, 4);

				i += 5;
				png.color_type = input[i];
				png.data_size = (png.width * png.height * get_byte_size_of_pixel(input[i]) + png.height);
				png.data = allocate(NULL, -1, png.data_size, sizeof(unsigned char)),

				i += 2;
				png.is_interlaced = input[i];

				i += 4;
			} else if (strncmp(chunk_name, "DAT", 3) == 0) {
				i += 3;

				const size_t sub_data_size = combine_bytes(input + i - 8, 4);
				data_size += sub_data_size;

				if (taken_headers == false) {
					png.zlib_headers[0] = input[i];
					png.zlib_headers[1] = input[i + 1];
				}

				data = allocate(data, -1, data_size, sizeof(unsigned char));
				memcpy(data + data_size - sub_data_size, input + i, sub_data_size);

				i += (sub_data_size + 4);
			} else if (strncmp(chunk_name, "END", 3) == 0) {
				z_stream infstream = {
					.zalloc = Z_NULL,
					.zfree = Z_NULL,
					.opaque = Z_NULL,
					.avail_in = data_size,
					.next_in = (Bytef *) data,
					.avail_out = png.data_size,
					.next_out = (Bytef *) png.data
				};

				inflateInit(&infstream);
				inflate(&infstream, Z_FINISH);
				inflateEnd(&infstream);
				free(data);
				break;
			}
		}
	}

	return png;
}

struct OutputPNG out_png(const struct PNG png) {
	struct OutputPNG output = {
		.data = allocate(NULL, -1, 57, sizeof(unsigned char))
	};

	/* Signature */
	const unsigned char signature[8] = {
		0x89, 0x50, 0x4E, 0x47,
		0x0D, 0x0A, 0x1A, 0x0A
	};

	memcpy(output.data, signature, sizeof(signature));
	/* /Signature */


	/* IHDR */
	// 8, 9 and 10 already are zero
	output.data[11] = 0x0D;
	output.data[12] = 0x49;
	output.data[13] = 0x48;
	output.data[14] = 0x44;
	output.data[15] = 0x52;

	// Width
	output.data[16] = ((png.width >> 24) & 0xFF);
	output.data[17] = ((png.width >> 16) & 0xFF);
	output.data[18] = ((png.width >> 8) & 0xFF);
	output.data[19] = (png.width & 0xFF);

	// Height
	output.data[20] = ((png.height >> 24) & 0xFF);
	output.data[21] = ((png.height >> 16) & 0xFF);
	output.data[22] = ((png.height >> 8) & 0xFF);
	output.data[23] = (png.height & 0xFF);

	// PNG properties
	output.data[24] = 8;
	output.data[25] = png.color_type;
	// 26 and 27 are already zero
	output.data[28] = png.is_interlaced;

	const unsigned long hdr_crc = crc32(0L, (output.data + 12), 17);
	output.data[29] = ((hdr_crc >> 24) & 0xFF);
	output.data[30] = ((hdr_crc >> 16) & 0xFF);
	output.data[31] = ((hdr_crc >> 8) & 0xFF);
	output.data[32] = (hdr_crc & 0xFF);
	/* /IHDR */


	/* IDAT */
	z_stream defstream = {
		.zalloc = Z_NULL,
		.zfree = Z_NULL,
		.opaque = Z_NULL,
		.avail_in = png.data_size,
		.next_in = (Bytef *) png.data,
		.avail_out = 0,
		.next_out = (Bytef *) NULL
	};

	size_t idat_size = deflateBound(&defstream, png.data_size);
	output.data = allocate(output.data, -1, 57 + idat_size, sizeof(unsigned char));

	defstream.avail_out = idat_size;
	defstream.next_out = (output.data + 41);

	deflateInit(&defstream, Z_DEFAULT_COMPRESSION);
	deflate(&defstream, Z_FINISH);
	deflateEnd(&defstream);

	if (defstream.total_out != idat_size) {
		output.data = allocate(output.data, -1, 57 + defstream.total_out, sizeof(unsigned char));
	}

	output.data_size = (57 + defstream.total_out);

	output.data[33] = ((defstream.total_out >> 24) & 0xFF);
	output.data[34] = ((defstream.total_out >> 16) & 0xFF);
	output.data[35] = ((defstream.total_out >> 8) & 0xFF);
	output.data[36] = (defstream.total_out & 0xFF);
	memcpy(output.data + 37, "IDAT", 4);

	const unsigned long dat_crc = crc32(0L, (output.data + 37), (4 + defstream.total_out));
	output.data[41 + defstream.total_out] = ((dat_crc >> 24) & 0xFF);
	output.data[42 + defstream.total_out] = ((dat_crc >> 16) & 0xFF);
	output.data[43 + defstream.total_out] = ((dat_crc >> 8) & 0xFF);
	output.data[44 + defstream.total_out] = (dat_crc & 0xFF);
	/* /IDAT */


	/* IEND */
	const unsigned char end[12] = {0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4E, 0x44, 0xAE, 0x42, 0x60, 0x82};
	memcpy(output.data + 45 + defstream.total_out, end, sizeof(end));
	/* /IEND */

	return output;
}
