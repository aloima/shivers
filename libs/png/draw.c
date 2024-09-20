#include <string.h>
#include <stdlib.h>

#include <png.h>

void set_pixel(struct PNG *png, const unsigned int x, const unsigned int y, const unsigned char *color, const unsigned int color_size) {
  if (!png->is_interlaced) {
    const unsigned int png_color_size = get_byte_size_of_pixel(png->color_type);
    const unsigned long start = ((y + 1) + (y * png->width * png_color_size) + (x * png_color_size));
    const unsigned int filter_method = png->data[(y * png->width * png_color_size) + y];
    unsigned char fix_color[png_color_size];
    memcpy(fix_color, color, color_size);

    if (png->color_type == PNG_RGBA_COLOR && color_size == 3) {
      fix_color[3] = 0xFF;
    }

    if (filter_method == 0) {
      if (png->color_type == PNG_RGBA_COLOR && color_size == 4) {
        const unsigned int alpha = color[3];
        const unsigned int diff = (255 - alpha);

        png->data[start] = ((png->data[start] * diff / 255) + (color[0] * alpha / 255));
        png->data[start + 1] = ((png->data[start + 1] * diff / 255) + (color[1] * alpha / 255));
        png->data[start + 2] = ((png->data[start + 2] * diff / 255) + (color[2] * alpha / 255));
      } else {
        for (unsigned int i = 0; i < png_color_size; ++i) {
          png->data[start + i] = fix_color[i];
        }
      }
    } // TODO: other filter methods for png parameter will be added.
  }
}

void draw_image(struct PNG *image, const struct PNG data, const unsigned int x, const unsigned int y, const bool as_circle) {
  const unsigned int data_color_size = get_byte_size_of_pixel(data.color_type);

  unsigned char *unfiltered_data, color[data_color_size];
  get_orig_data(data, &unfiltered_data);

  if (as_circle && (data.width == data.height)) {
    const unsigned int radius = (data.width / 2);
    const unsigned int rr = (radius * radius);

    for (unsigned int a = 0; a < data.width; ++a) {
      const unsigned int aa = ((a - radius) * (a - radius));

      for (unsigned int b = 0; b < data.height; ++b) {
        if ((aa + ((b - radius) * (b - radius))) <= rr) {
          get_pixel_from_data(data, unfiltered_data, a, b, color);
          set_pixel(image, x + a, y + b, color, data_color_size);
        }
      }
    }
  } else {
    for (unsigned int a = 0; a < data.width; ++a) {
      for (unsigned int b = 0; b < data.height; ++b) {
        get_pixel_from_data(data, unfiltered_data, a, b, color);
        set_pixel(image, x + a, y + b, color, data_color_size);
      }
    }
  }

  free(unfiltered_data);
}

void draw_circle(struct PNG *png, const struct Circle circle, const unsigned int x, const unsigned int y) {
  const unsigned int rr = (circle.radius * circle.radius);
  const unsigned int a_limit = (x + circle.radius);
  const unsigned int b_limit = (y + circle.radius);

  if (circle.fill) {
    for (unsigned int a = 0; a < a_limit; ++a) {
      const unsigned int aa = ((a - circle.radius) * (a - circle.radius));

      for (unsigned int b = 0; b < b_limit; ++b) {
        if ((aa + ((b - circle.radius) * (b - circle.radius))) <= rr) {
          set_pixel(png, a + x - circle.radius, b + y - circle.radius, circle.color, circle.color_size);
        }
      }
    }
  }
}

void draw_rect(struct PNG *png, const struct Rectangle rectangle, const unsigned int x, const unsigned int y) {
  if (rectangle.fill) {
    if (rectangle.border_radius != 0) {
      const unsigned int rr = (rectangle.border_radius * rectangle.border_radius);

      for (unsigned int a = 0; a < rectangle.border_radius; ++a) {
        const unsigned int lta = (rectangle.border_radius - a);
        const unsigned int lba = lta;
        const unsigned int rta = a;
        const unsigned int rba = a;

        for (unsigned int b = 0; b < rectangle.border_radius; ++b) {
          const unsigned int ltb = (rectangle.border_radius - b);
          const unsigned int lbb = b;
          const unsigned int rtb = ltb;
          const unsigned int rbb = b;

          if (((lta * lta) + (ltb * ltb)) <= rr) {
            set_pixel(png, x + a, y + b, rectangle.color, rectangle.color_size);
          }

          if (((lba * lba) + (lbb * lbb)) <= rr) {
            set_pixel(png, x + a, y + rectangle.height - rectangle.border_radius + b, rectangle.color, rectangle.color_size);
          }

          if (((rta * rta) + (rtb * rtb)) <= rr) {
            set_pixel(png, x + rectangle.width - rectangle.border_radius + a, y + b, rectangle.color, rectangle.color_size);
          }

          if (((rba * rba) + (rbb * rbb)) <= rr) {
            set_pixel(png, x + rectangle.width - rectangle.border_radius + a, y + rectangle.height - rectangle.border_radius + b, rectangle.color, rectangle.color_size);
          }
        }
      }

      struct Rectangle temp = {
        .border_radius = 0,
        .color = rectangle.color,
        .color_size = rectangle.color_size,
        .fill = rectangle.fill,
        .width = rectangle.width - rectangle.border_radius * 2,
        .height = rectangle.height,
      };

      draw_rect(png, temp, x + rectangle.border_radius, y);

      temp.width = rectangle.border_radius;
      temp.height = rectangle.height - rectangle.border_radius * 2;
      draw_rect(png, temp, x, y + rectangle.border_radius);
      draw_rect(png, temp, x + rectangle.width - rectangle.border_radius, y + rectangle.border_radius);
    } else {
      const unsigned int max_y = (rectangle.height + y);
      const unsigned int max_x = (rectangle.width + x);

      for (unsigned int j = y; j < max_y; ++j) {
        for (unsigned int i = x; i < max_x; ++i) {
          set_pixel(png, i, j, rectangle.color, rectangle.color_size);
        }
      }
    }
  }
}
