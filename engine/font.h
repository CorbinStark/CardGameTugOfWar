///////////////////////////////////////////////////////////////////////////
// FILE:                        font.h                                   //
///////////////////////////////////////////////////////////////////////////
//                      BAHAMUT GRAPHICS LIBRARY                         //
//                        Author: Corbin Stark                           //
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2019 Corbin Stark                                       //
//                                                                       //
// Permission is hereby granted, free of charge, to any person obtaining //
// a copy of this software and associated documentation files (the       //
// "Software"), to deal in the Software without restriction, including   //
// without limitation the rights to use, copy, modify, merge, publish,   //
// distribute, sublicense, and/or sell copies of the Software, and to    //
// permit persons to whom the Software is furnished to do so, subject to //
// the following conditions:                                             //
//                                                                       //
// The above copyright notice and this permission notice shall be        //
// included in all copies or substantial portions of the Software.       //
//                                                                       //
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       //
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    //
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.//
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  //
// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  //
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     //
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                //
///////////////////////////////////////////////////////////////////////////

#ifndef FONT_H
#define FONT_H

#include "defines.h"
#include "texture.h"
#include "maths.h"
#include <ft2build.h>
#include FT_FREETYPE_H 
#include <stdexcept>

struct Character {
	Texture	texture;
	vec2	size;
	vec2	bearing;
	GLuint	advance;
};

struct Font {
	Character** characters;
	FT_Face face;
	FT_Library ft;
	int size;
};

static inline
Character* get_char(Font& font, const GLchar c) {
	try {
		return font.characters[c];
	}
	catch (std::out_of_range) {
		printf("Glyph '%c' is not valid\n", c);
	}
}

static inline
f32 get_font_height(Font& font) {
	return get_char(font, 'T')->bearing.y + (get_char(font, 'T')->size.y / 2);
}

static inline
Texture create_texture_from_string(Font& font, std::vector<u64>& str, i32 texParam, GLubyte r, GLubyte g, GLubyte b) {
	unsigned int w = 0;
	unsigned int h = 0;

	//get width and height of texture
	for (auto i = 0; i < str.size(); ++i) {
		if (FT_Load_Char(font.face, str[i], FT_LOAD_RENDER)) {
			continue;
		}
		w += (font.face->glyph->advance.x >> 6);
		h = get_font_height(font);
		if (str[i] == '\n') h += get_font_height(font);
	}
	h = 150;
	w += (font.face->glyph->advance.x >> 6);
	w += (font.face->glyph->advance.x >> 6);

	//gen texture
	Texture tex;
	tex.width = w;
	tex.height = h;
	tex.flip_flag = 0;

	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &tex.ID);
	glBindTexture(GL_TEXTURE_2D, tex.ID);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	//blank texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texParam);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texParam);

	int x = 0;
	int currentY = 0;
	bool xStart = false;
	for (auto i = 0; i < str.size(); ++i) {
		if (str[i] == '\n') {
			x = 0;
			currentY += get_font_height(font);
			xStart = true;
			continue;
		}
		if (FT_Load_Char(font.face, str[i], FT_LOAD_RENDER))
			continue;

		int yOffset = (get_char(font, 'T')->bearing.y - font.face->glyph->bitmap_top) + 1;
		if (yOffset < 0) yOffset = 0;

        int width = font.face->glyph->bitmap.width;
        int height = font.face->glyph->bitmap.rows;
        GLubyte* glyphPixels = font.face->glyph->bitmap.buffer;
        //convert the pixels from alpha only to RGBA
        GLubyte* pixels = (GLubyte*)malloc(width * height * 4);
        if (glyphPixels != NULL) {
            int j = 0;
            for (int i = 0; i < width * height * 4; ++i) {
                pixels[i++] = 255; //red
                pixels[i++] = 255; //green
                pixels[i++] = 255; //blue
                pixels[i] = glyphPixels[j++]; //alpha
            }
        }

		if (xStart) {
			//if we hit a newline go back to writing characters at the beginning of the xPos
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, yOffset + currentY, font.face->glyph->bitmap.width,
				font.face->glyph->bitmap.rows, GL_RGBA, GL_UNSIGNED_BYTE, pixels
			);
			xStart = false;
		}
		else {
			glTexSubImage2D(GL_TEXTURE_2D, 0, x + font.face->glyph->bitmap_left, yOffset + currentY, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
		}

		//advance current x by one character to the right
		x += (font.face->glyph->advance.x >> 6);

		delete[] pixels;
	}

	glBindTexture(GL_TEXTURE_2D, 0);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	return tex;
}

static inline
Texture create_texture_from_string(Font& font, std::vector<u64>& str, i32 texParam) {
	return create_texture_from_string(font, str, texParam, 0, 0, 0);
}

static inline
void load_character(Font& font, unsigned int size, i32 texParam, u64 c) {
    if (FT_Load_Char(font.face, c, FT_LOAD_RENDER)) {
        printf("Failed to load Glyph\n");
        return;
    }

    Character* character = new Character;

    glGenTextures(1, &character->texture.ID);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, character->texture.ID);

    character->texture.width = font.face->glyph->bitmap.width;
    character->texture.height = font.face->glyph->bitmap.rows;
    character->texture.flip_flag = 0;

    GLubyte* glyphPixels = font.face->glyph->bitmap.buffer;

    //convert the pixels from alpha only to RGBA
    GLubyte* pixels = (GLubyte*)malloc(character->texture.width * character->texture.height * 4);
    if (glyphPixels != NULL) {
        int j = 0;
        for (int i = 0; i < character->texture.width * character->texture.height * 4; ++i) {
            pixels[i++] = 255; //red
            pixels[i++] = 255; //green
            pixels[i++] = 255; //blue
            pixels[i] = glyphPixels[j++]; //alpha
        }
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, character->texture.width, character->texture.height, 0,
            GL_RGBA, GL_UNSIGNED_BYTE, pixels
            );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texParam);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texParam);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glBindTexture(GL_TEXTURE_2D, 0);

    character->size = V2((float)font.face->glyph->bitmap.width, (float)font.face->glyph->bitmap.rows);
    character->bearing = V2((float)font.face->glyph->bitmap_left, (float)font.face->glyph->bitmap_top);
    character->advance = font.face->glyph->advance.x;

    free(pixels);

    font.characters[c] = character;
}

static inline
Font load_font(const GLchar* filepath, unsigned int size, i32 texParam) {
	Font font;
	font.size = size;
    font.characters = new Character*[ 0xFFFF ];
    for(u64 i = 0; i < 0xFFFF; ++i)
        font.characters[i] = NULL;

	//load lib
	if (FT_Init_FreeType(&font.ft)) {
		printf("Could not initialize FreeType font\n");
		font = { 0 };
		return font;
	}

	//load face
	if (FT_New_Face(font.ft, filepath, 0, &font.face)) {
		printf("Failed to load font '%s'\n", filepath);
		font = { 0 };
		return font;
	}
	FT_Set_Pixel_Sizes(font.face, 0, size);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	//store char textures
    //English chars
	for (u64 c = 32; c < 128; ++c)
        load_character(font, size, texParam, c);
    //Hiragana
    for (u64 c = 0x3041; c < 0x309F; ++c)
        load_character(font, size, texParam, c);
    //Katakana
    for (u64 c = 0x30A0; c < 0x30FF; ++c)
        load_character(font, size, texParam, c);
    //Katakana Phonetic Extensions
    for (u64 c = 0x31F0; c < 0x31FF; ++c)
        load_character(font, size, texParam, c);
    //Kanbun
    for (u64 c = 0x3190; c < 0x319F; ++c)
        load_character(font, size, texParam, c);
    //Halfwidth and fullwidth forms
    for (u64 c = 0xFF00; c < 0xFFEF; ++c)
        load_character(font, size, texParam, c);
    //Kanji
    for (u64 c = 0x4E00; c < 0x9FAF; ++c)
        load_character(font, size, texParam, c);

	//reset gl_unpack_alignment
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	return font;
}

static inline
void dispose_font(Font& font) {
	if (font.face == NULL) {
		BMT_LOG(WARNING, "Font face is NULL");
		return;
	}
	if (font.characters == NULL) {
		BMT_LOG(WARNING, "Font characters are NULL");
	}

	for (u64 i = 0; i < 0xFFFF; ++i) {
        if(font.characters[i] != NULL) {
            dispose_texture(font.characters[i]->texture);
            delete font.characters[i];
        }
	}
    delete[] font.characters;

	FT_Done_Face(font.face);
	FT_Done_FreeType(font.ft);
}

static inline 
const u32 get_string_width(Font& font, const char* str) {
	u32 len = strlen(str);
	u32 width = 0;
	for (u32 i = 0; i < len; ++i) {
		if (str[i] == '\n') return width;
		width += (font.characters[str[i]]->advance >> 6);
	}
	return width;
}

//text must be less than 128 chars long.
static inline 
const char* format_text(const char* text, ...) {
	static char buffer[MAX_FORMAT_TEXT_SIZE];

	va_list args;
	va_start(args, text);
	vsprintf(buffer, text, args);
	va_end(args);

	return buffer;
}
#endif
