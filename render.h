#ifndef RENDER_H
#define RENDER_H

#include <vector>
#include <unordered_map>
#include "engine/bahamut.h"

static inline
u32 rgba_to_u32(u8 r, u8 g, u8 b, u8 a) {
	return ( (r & 0xFF) << 24) + ( (g & 0xFF) << 16) + ( (b & 0xFF) << 8) + ( (a & 0xFF) ); 
}

static inline
vec4 u32_to_rgba(u32 color) {
    vec4 c;
    c.x = ( (color >> 24) & 0xFF);
    c.y = ( (color >> 16) & 0xFF);
    c.z = ( (color >>  8) & 0xFF);
    c.w = ( (color)       & 0xFF);
    return c;
}

#define UnicodeStr std::vector<u64>
#define StringList std::vector< UnicodeStr >
#include <fstream>

StringList get_lines_from_file(const char* filepath);
void load_ninepatch(const char* path, Texture ninepatch[9]);
void draw_ninepatch(QuadBatch* batch, Texture ninepatch[9], i32 xPos, i32 yPos, u16 width, u16 height);
void pre_render_text(Font& font, QuadBatch& batch, StringList& lines, std::vector<Texture>& result);
int get_string_width(Font& font, UnicodeStr& str);


#endif
