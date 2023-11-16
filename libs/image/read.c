#include <zlib.h>

#include <network.h>
#include <image.h>
#include <utils.h>

struct Image read_png(const unsigned char *data, const size_t size) {
	struct Image image;

	for (size_t i = 0; i < size; ++i) {
		const unsigned char ch = data[i];

		if (ch == 'I') {
			++i;

			if (strncmp((char *) data + i, "HDR", 3) == 0) {
				i += 3;
				const unsigned int width = combine_bytes((unsigned char *) data + i, 4);

				i += 4;
				const unsigned int height = combine_bytes((unsigned char *) data + i, 4);

				image = create_image(width, height);
			} else if (strncmp((char *) data + i, "DAT", 3) == 0) {
				const size_t compressed_size = combine_bytes((unsigned char *) data + i - 5, 4);
				i += 3;

				const unsigned long output_size = (image.width * image.height * 3 + image.height);

				z_stream infstream = {
					.zalloc = Z_NULL,
					.zfree = Z_NULL,
					.opaque = Z_NULL,
					.avail_in = compressed_size,
					.next_in = (Bytef *) data + i,
					.avail_out = output_size,
					.next_out = (Bytef *) image.data
				};

				inflateInit(&infstream);
				inflate(&infstream, Z_FINISH);
				inflateEnd(&infstream);

				break;
			}
		}
	}

	return image;
}
