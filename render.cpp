#include "render.h"

std::wstring convert_UTF8(const char* str)
{
    const unsigned char* s = reinterpret_cast<const unsigned char*>(str);

    static const wchar_t badchar = '?';

    std::wstring ret;

    unsigned i = 0;
    while(s[i])
    {
        try
        {
            if(s[i] < 0x80)         // 00-7F: 1 byte codepoint
            {
                ret += s[i];
                ++i;
            }
            else if(s[i] < 0xC0)    // 80-BF: invalid for midstream
                throw 0;
            else if(s[i] < 0xE0)    // C0-DF: 2 byte codepoint
            {
                if((s[i+1] & 0xC0) != 0x80)		throw 1;

                ret +=  ((s[i  ] & 0x1F) << 6) |
                        ((s[i+1] & 0x3F));
                i += 2;
            }
            else if(s[i] < 0xF0)    // E0-EF: 3 byte codepoint
            {
                if((s[i+1] & 0xC0) != 0x80)		throw 1;
                if((s[i+2] & 0xC0) != 0x80)		throw 2;

                wchar_t ch = 
                        ((s[i  ] & 0x0F) << 12) |
                        ((s[i+1] & 0x3F) <<  6) |
                        ((s[i+2] & 0x3F));
                i += 3;

                // make sure it isn't a surrogate pair
                if((ch & 0xF800) == 0xD800)
                    ch = badchar;

                ret += ch;
            }
            else if(s[i] < 0xF8)    // F0-F7: 4 byte codepoint
            {
                if((s[i+1] & 0xC0) != 0x80)		throw 1;
                if((s[i+2] & 0xC0) != 0x80)		throw 2;
                if((s[i+3] & 0xC0) != 0x80)		throw 3;

                unsigned long ch = 
                        ((s[i  ] & 0x07) << 18) |
                        ((s[i+1] & 0x3F) << 12) |
                        ((s[i+2] & 0x3F) <<  6) |
                        ((s[i+3] & 0x3F));
                i += 4;

                // make sure it isn't a surrogate pair
                if((ch & 0xFFF800) == 0xD800)
                    ch = badchar;

                if(ch < 0x10000)	// overlong encoding -- but technically possible
                    ret += static_cast<wchar_t>(ch);
                else if(std::numeric_limits<wchar_t>::max() < 0x110000)
                {
                    // wchar_t is too small for 4 byte code point
                    //  encode as UTF-16 surrogate pair

                    ch -= 0x10000;
                    ret += static_cast<wchar_t>( (ch >> 10   ) | 0xD800 );
                    ret += static_cast<wchar_t>( (ch & 0x03FF) | 0xDC00 );
                }
                else
                    ret += static_cast<wchar_t>(ch);
            }
            else                    // F8-FF: invalid
                throw 0;
        }
        catch(int skip)
        {
            if(!skip)
            {
                do
                {
                    ++i;
                }while((s[i] & 0xC0) == 0x80);
            }
            else
                i += skip;
        }
    }

    return ret;
}

StringList get_lines_from_file(const char* filepath) {
    StringList lines;
    std::string fullpath = "data/language/";
    std::ifstream file( fullpath.append(filepath) );

    std::string utf8;
    while( std::getline(file, utf8) ) {
        UnicodeStr str;
        std::wstring line = convert_UTF8( utf8.c_str() );
        for(int i = 0; i < line.size(); ++i) {
	    if(line[i] != 13)
                str.push_back(line[i]);
        }
        lines.push_back( str );
    }
    return lines;
}

static inline 
Texture get_sub_image(unsigned char* pixels, i32 pixels_width, i32 x, i32 y, i32 width, i32 height, i32 texparam) {
    Texture subimage;
    glPixelStorei(GL_UNPACK_ROW_LENGTH, pixels_width);
    unsigned char* subimage_pixels = pixels + (x + y * pixels_width) * 4;
    subimage = load_texture(subimage_pixels, width, height, texparam);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    return subimage;
}

void draw_ninepatch(QuadBatch* batch, Texture ninepatch[9], i32 xPos, i32 yPos, u16 width, u16 height) {
    assert(width >= 1);
    assert(height >= 1);

    draw_texture(batch, ninepatch[0], xPos, yPos);
    draw_texture(batch, ninepatch[2], xPos, yPos + (height * ninepatch[0].height));
    draw_texture(batch, ninepatch[6], xPos + (width * ninepatch[0].width), yPos);
    draw_texture(batch, ninepatch[8], xPos + (width * ninepatch[0].width), yPos + (height * ninepatch[0].height));

    for (int x = 1; x < width; ++x) {
        draw_texture(batch, ninepatch[3], (x * ninepatch[0].width) + xPos, yPos);
        draw_texture(batch, ninepatch[5], (x * ninepatch[0].width) + xPos, yPos + (height * ninepatch[0].height));
    }
    for (int y = 1; y < height; ++y) {
        draw_texture(batch, ninepatch[1], xPos, (y * ninepatch[0].height) + yPos);
        draw_texture(batch, ninepatch[7], xPos + (width * ninepatch[0].width), (y * ninepatch[0].height) + yPos);
    }
    for (int x = 1; x < width; ++x) {
        for (int y = 1; y < height; ++y) {
            draw_texture(batch, ninepatch[4], (x * ninepatch[0].width) + xPos, (y * ninepatch[0].height) + yPos);
        }
    }
}

void load_ninepatch(const char* path, Texture ninepatch[9]) {
    i32 w;
    i32 h;
    u16 i = 0;
    unsigned char* image = SOIL_load_image(path, &w, &h, 0, SOIL_LOAD_RGBA);
    for (u8 x = 0; x < 3; ++x)
        for (u8 y = 0; y < 3; ++y)
            ninepatch[i++] = get_sub_image(image, w, x * (w / 3), y * (h / 3), w / 3, h / 3, GL_LINEAR);
    free(image);
}

void pre_render_text(Font& font, QuadBatch& batch, StringList& lines, std::vector<Texture>& result) {
    set_clear_color(255, 255, 255, 0);
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    set_viewport(0, 0, 1920, 1080);
    upload_mat4(batch.shader, "projection", orthographic_projection(0, 0, 1920, 1080, -1, 1));
    for( int i = 0; i < lines.size(); ++i ) {
        int width = get_string_width(font, lines[i]);
        Framebuffer buffer = create_color_buffer(width, 50, GL_LINEAR);

        bind_framebuffer(buffer);
        clear_bound_framebuffer();
        bind_quad_batch(&batch);
        {
            draw_text(&batch, font, lines[i], 0, 1080 - 10);
        }
        unbind_quad_batch(&batch);
        unbind_framebuffer();

        glDeleteFramebuffers(1, &buffer.ID);

        buffer.texture.flip_flag = FLIP_VERTICAL;
        result.push_back(buffer.texture);
    }
    set_viewport(0, 0, get_window_width(), get_window_height());
    upload_mat4(batch.shader, "projection", orthographic_projection(0, 0, get_window_width(), get_window_height(), -1, 1));
    set_clear_color(0, 0, 0, 255);
}

int get_string_width(Font& font, UnicodeStr& str) {
    u32 width = 0;
    for (u32 i = 0; i < str.size() ; ++i) {
        if(str[i] == 13) return width;
	if(str[i] == 10) return width;
	if(str[i] == '\n') return width;
        width += (font.characters[str[i]]->advance >> 6);
    }
    return width;
}
