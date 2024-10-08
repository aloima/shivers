#include <string.h>
#include <stdlib.h>

#include <zconf.h>
#include <zlib.h>

#include <png.h>
#include <network.h>
#include <utils.h>

void initialize_png(struct PNG *png) {
  const unsigned char color_size = get_byte_size_of_pixel(png->color_type);

  png->data_size = (png->width * png->height * color_size + png->height);
  png->data = allocate(NULL, 0, png->data_size, sizeof(unsigned char));

  if (png->color_type == PNG_RGBA_COLOR) {
    for (unsigned int x = 0; x < png->width; ++x) {
      for (unsigned int y = 0; y < png->height; ++y) {
        png->data[((y + 1) + ((y * png->width) + x) * color_size) + 3] = 0xFF;
      }
    }
  }
}

struct PNG read_png(unsigned char *input, const unsigned long input_size) {
  struct PNG png = {0};

  unsigned char *data = NULL;
  unsigned long data_size = 0;
  char *current = (char *) input;

  while (true) {
    if (strncmp(current, "IHDR", 4) == 0) {
      current += 4;
      png.width = combine_bytes((unsigned char *) current, 4);

      current += 4;
      png.height = combine_bytes((unsigned char *) current, 4);

      current += 5;
      png.color_type = *current;

      switch (png.color_type) {
        case PNG_PALETTE_COLOR: {
          png.data_size = ((png.width * png.height) + png.height);
          break;
        }

        default:
          png.data_size = (png.width * png.height * get_byte_size_of_pixel(png.color_type) + png.height);
      }

      png.data = allocate(NULL, -1, png.data_size, sizeof(unsigned char)),

      current += 2;
      png.is_interlaced = *current;

      current += 4;
    } else if (strncmp(current, "PLTE", 4) == 0) {
      png.palette_size = combine_bytes((unsigned char *) current - 4, 4);
      png.palette = allocate(NULL, -1, png.palette_size, sizeof(unsigned char));
      current += 4;
      memcpy(png.palette, current, png.palette_size);
      current += (png.palette_size + 4);
    } else if (strncmp(current, "IDAT", 4) == 0) {
      const unsigned long sub_data_size = combine_bytes((unsigned char *) current - 4, 4);
      data_size += sub_data_size;
      current += 4;

      data = allocate(data, -1, data_size, sizeof(unsigned char));
      memcpy(data + data_size - sub_data_size, current, sub_data_size);

      current += (sub_data_size + 4);
    } else if (strncmp(current, "IEND", 4) == 0) {
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

    ++current;
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

  unsigned long idat_size = deflateBound(&defstream, png.data_size);
  output.data = allocate(output.data, -1, 57 + idat_size, sizeof(unsigned char));

  defstream.avail_out = idat_size;
  defstream.next_out = (output.data + 41);

  deflateInit2(&defstream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 9, 4, Z_DEFAULT_STRATEGY);
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
