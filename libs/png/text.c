#include <ft2build.h>
#include FT_FREETYPE_H

#include <png.h>
#include <utils.h>

static FT_Library library;
static struct Fonts fonts;

void initialize_fonts() {
	if (FT_Init_FreeType(&library)) {
		throw("initialize_font_library(): library initialization problem");
	}

	if (FT_New_Face(library, "assets/arial.ttf", 0, &fonts.arial)) {
		FT_Done_FreeType(library);
		throw("initialize_font_library(): arial font initialization problem");
	}
}

struct Fonts get_fonts() {
	return fonts;
}

void free_fonts() {
	FT_Done_Face(fonts.arial);
	FT_Done_FreeType(library);
}

void write_text(struct PNG *png, unsigned int x, unsigned int y, const char *text, FT_Face font, const unsigned char *color, const unsigned char size) {
	FT_Set_Pixel_Sizes(font, 0, 16);
	FT_Set_Char_Size(font, 0, size * 64, 300, 300);

	unsigned int i = 0;
	char ch = text[i];
	unsigned int padding = 0;
	unsigned char font_color[4] = {color[0], color[1], color[2], 0};

	while (ch != 0) {
		FT_Load_Char(font, ch, FT_LOAD_RENDER);
		FT_Bitmap bitmap = font->glyph->bitmap;
		unsigned char baseline = (font->glyph->metrics.horiBearingY >> 6);
		unsigned char padding_y = (bitmap.rows - baseline);

		for (int h = 0; h < bitmap.rows; ++h) {
			for (int w = 0; w < bitmap.width; ++w) {
				unsigned char char_data = bitmap.buffer[w + (h * bitmap.width)];
				font_color[3] = char_data;

				if (char_data != 0) {
					set_pixel(png, (x + padding + w), (y + padding_y - (bitmap.rows - h)), font_color, 4);
				}
			}
		}

		++i;
		padding += (bitmap.width + 2);
		ch = text[i];
	}
}
