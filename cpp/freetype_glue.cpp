/*
    Copyright © 2020 endorfina <dev.endorfina@outlook.com>

    This file is part of Idle.

    Idle is free software: you can study it, redistribute it
    and/or modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation, either version 3 of
    the License, or (at your option) any later version.

    Idle is distributed in the hope that it will be fun and useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Idle. If not, see <http://www.gnu.org/licenses/>.
*/

#include <memory>

#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <freetype/ftoutln.h>
#include <freetype/fttrigon.h>

#include "freetype_glue.hpp"
#include <log.hpp>
#include "drawable.hpp"

namespace fonts
{

namespace
{

template <typename T>
std::enable_if_t<std::is_integral_v<T>, T> next_power_of_2(T a) {
    T rval = 1;
    while (rval < a)
        rval *= 2;
    return rval;
}

void set_pixel(unsigned char *texture, int offset, int size, int x, int y, unsigned char val) {
    texture[offset + x + y * size] = val % 0xff == 0 ? val : static_cast<unsigned char>(sinf(static_cast<float>(val) * F_TAU_4 / 255) * 255);
}

using font_face_t = std::unique_ptr<std::remove_pointer_t<FT_Face>, decltype(&FT_Done_Face)>;

std::optional<font_t> create_font(font_face_t freetype_font_face, const int resolution, const int cell_margin)
{
    FT_Set_Pixel_Sizes(freetype_font_face.get(), resolution, resolution);

    const int character_count = freetype_font_face->num_glyphs;
    const int cell_size = resolution + cell_margin * 2;
    const int character_row_width = (int)ceilf(sqrt(character_count));
    const int texSize = character_row_width * cell_size;
    const int actual_texture_size = next_power_of_2(texSize);

    auto texture_data = std::make_unique<unsigned char[]>(actual_texture_size * actual_texture_size);

    font_t::map_t glyphs;

    math::point2<int> texture_position{0, 0};


    FT_UInt gindex;
    for (FT_ULong charcode = FT_Get_First_Char(freetype_font_face.get(), &gindex); !!gindex;
                charcode = FT_Get_Next_Char(freetype_font_face.get(), charcode, &gindex))
    if (!FT_Load_Glyph(freetype_font_face.get(), gindex, FT_LOAD_DEFAULT))
    {
        FT_GlyphSlot glyph = freetype_font_face->glyph;
        font_t::glyph_t g;
        FT_Render_Glyph(glyph, FT_RENDER_MODE_NORMAL);

        // Calculate glyph informations

        g.offset = idle::point_t{ glyph->bitmap_left / static_cast<float>(cell_size),
                -(glyph->bitmap_top / static_cast<float>(cell_size)) };

        g.texture_position = idle::point_t{ texture_position.x * cell_size / static_cast<float>(actual_texture_size),
                (texture_position.y * cell_size + 1) / static_cast<float>(actual_texture_size) };

        g.width = (glyph->advance.x) / static_cast<float>(64.0f * cell_size);

        glyphs.emplace(charcode, std::move(g));

        const int base_offset = texture_position.x * cell_size + texture_position.y * cell_size * actual_texture_size;
        const int gr = glyph->bitmap.rows;
        const int gw = glyph->bitmap.width;

        for (int h = 0; h < gr; ++h)
            for (int w = 0; w < gw; ++w)
                set_pixel(texture_data.get(), base_offset + cell_margin, actual_texture_size, w, cell_margin + h,
                        glyph->bitmap.buffer[w + h * gw]);

        for (int h = 0; h < cell_size; ++h) {
            if (h < cell_margin || h >= cell_margin + gr) {
                // bottom padding => whole rows
                for (int w = 0; w < cell_size; ++w) {
                    set_pixel(texture_data.get(), base_offset, actual_texture_size, w, h, 0);
                }
            } else {
                // left padding
                for (int w = 0; w < cell_margin; ++w)
                    set_pixel(texture_data.get(), base_offset, actual_texture_size, w, h, 0);
                // right padding
                for (int w = gw + cell_margin; w < cell_size; ++w)
                    set_pixel(texture_data.get(), base_offset, actual_texture_size, w, h, 0);
            }
        }

        if (++texture_position.x >= character_row_width) {
            texture_position.x = 0;
            ++texture_position.y;
        }
    }

    return font_t{ idle::image_t::load_from_memory(
                actual_texture_size, actual_texture_size,
#ifdef __ANDROID__
                gl::LUMINANCE, gl::LUMINANCE,
#else
                gl::R8, gl::RED,
#endif
                gl::LINEAR, gl::CLAMP_TO_EDGE, std::move(texture_data)).release(),
            std::move(glyphs), cell_size / (float)actual_texture_size };
}

}  // namespace


std::optional<font_t> freetype_glue::load(const std::string_view &memory, int resolution) const
{
    const int margin = static_cast<int>(ceilf(resolution / 10.f));
    return load(memory, resolution, margin);
}


static FT_Library library;

std::optional<font_t> freetype_glue::load(const std::string_view &memory, int resolution, int cell_margin) const
{
    if (FT_Face ff = nullptr;
            !FT_New_Memory_Face(
                library,
                reinterpret_cast<const FT_Byte *>(memory.data()),
                static_cast<FT_Long>(memory.size()), 0, &ff))
    {
        return create_font({ ff, FT_Done_Face }, resolution, cell_margin);
    }

    LOGE("Error loading font face");
    return {};
}

static unsigned library_ref_count = 0;

freetype_glue::freetype_glue()
{
    if (++library_ref_count == 1 && FT_Init_FreeType(&library))
    {
        LOGE("Failed to initialize freetype library");
    }
}

freetype_glue::~freetype_glue()
{
    if (library_ref_count && !--library_ref_count)
    {
        FT_Done_FreeType(library);
    }
}

}  // namespace fonts
