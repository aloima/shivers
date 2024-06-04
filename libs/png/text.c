#include <ft2build.h>
#include FT_FREETYPE_H

#include <png.h>
#include <utils.h>

static FT_Library library;
static struct Fonts fonts;

#define CHAR_SPACE 2

void initialize_fonts() {
	if (FT_Init_FreeType(&library)) {
		throw("initialize_font_library(): library initialization problem");
	}

	if (FT_New_Face(library, "assets/arial.ttf", 0, &fonts.arial)) {
		FT_Done_FreeType(library);
		throw("initialize_font_library(): arial font initialization problem");
	}

	if (FT_New_Face(library, "assets/quicksand.ttf", 0, &fonts.quicksand)) {
		FT_Done_FreeType(library);
		throw("initialize_font_library(): arial font initialization problem");
	}
}

struct Fonts get_fonts() {
	return fonts;
}

void free_fonts() {
	FT_Done_Face(fonts.arial);
	FT_Done_Face(fonts.quicksand);
	FT_Done_FreeType(library);
}

unsigned long print_char(struct PNG *png, unsigned int x, unsigned int y, const char ch, FT_Face font, const unsigned int padding_x, const unsigned char *color) {
	unsigned char font_color[4] = {color[0], color[1], color[2], 0};

	FT_Load_Char(font, ch, FT_LOAD_RENDER);
	FT_Bitmap bitmap = font->glyph->bitmap;
	unsigned char baseline = (font->glyph->metrics.horiBearingY >> 6);
	unsigned char padding_y = (bitmap.rows - baseline);

	for (int h = 0; h < bitmap.rows; ++h) {
		for (int w = 0; w < bitmap.width; ++w) {
			unsigned char char_data = bitmap.buffer[w + (h * bitmap.width)];
			font_color[3] = char_data;

			if (char_data != 0) {
				set_pixel(png, (x + padding_x + w), (y + padding_y - (bitmap.rows - h)), font_color, 4);
			}
		}
	}

	return bitmap.width;
}

void write_text(struct PNG *png, unsigned int x, unsigned int y, const char *text, FT_Face font, const unsigned char *color, const unsigned char size, const unsigned char text_alignment) {
	FT_Set_Pixel_Sizes(font, 0, 16);
	FT_Set_Char_Size(font, 0, size * 64, 300, 300);

	unsigned int i = 0;
	char ch = text[i];
	unsigned int padding_x = 0;

	switch (text_alignment) {
		case PNG_TEXT_LEFT: {
			while (ch != 0) {
				if (ch != ' ') {
					const unsigned long width = print_char(png, x, y, ch, font, padding_x, color);
					padding_x += (width + CHAR_SPACE);
				} else {
					padding_x += (size + CHAR_SPACE);
				}

				++i;
				ch = text[i];
			}

			break;
		}

		case PNG_TEXT_CENTER: {
			unsigned long right_start_x = x;

			while (ch != 0) {
				if (ch == ' ') {
					right_start_x -= (size + CHAR_SPACE);
				} else {
					FT_Load_Char(font, ch, FT_LOAD_RENDER);
					FT_Bitmap bitmap = font->glyph->bitmap;

					right_start_x -= (bitmap.width + CHAR_SPACE);
				}

				++i;
				ch = text[i];
			}

			i = 0;
			ch = text[i];

			while (ch != 0) {
				if (ch != ' ') {
					const unsigned long width = print_char(png, x - ((x - right_start_x) / 2), y, ch, font, padding_x, color);
					padding_x += (width + CHAR_SPACE);
				} else {
					padding_x += (size + CHAR_SPACE);
				}

				++i;
				ch = text[i];
			}

			break;
		}


		case PNG_TEXT_RIGHT: {
			unsigned long start_x = x;

			while (ch != 0) {
				if (ch == ' ') {
					start_x -= (size + CHAR_SPACE);
				} else {
					FT_Load_Char(font, ch, FT_LOAD_RENDER);
					FT_Bitmap bitmap = font->glyph->bitmap;

					start_x -= (bitmap.width + CHAR_SPACE);
				}

				++i;
				ch = text[i];
			}

			i = 0;
			ch = text[i];

			while (ch != 0) {
				if (ch != ' ') {
					const unsigned long width = print_char(png, start_x, y, ch, font, padding_x, color);
					padding_x += (width + CHAR_SPACE);
				} else {
					padding_x += (size + CHAR_SPACE);
				}

				++i;
				ch = text[i];
			}

			break;
		}
	}
}
