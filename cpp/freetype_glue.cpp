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

#include <cmath>
#include <memory>

#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <freetype/ftoutln.h>
#include <freetype/fttrigon.h>

#include "freetype_glue.hpp"
#include <log.hpp>

namespace fonts
{

namespace
{

template <typename T>
std::enable_if_t<std::is_integral_v<T>, T> next_power_of_2(const T a) {
    T rval = 1;
    while (rval < a)
        rval *= 2;
    return rval;
}

unsigned char fine(unsigned char val)
{
    return static_cast<unsigned char>(std::sin(static_cast<float>(val) * F_TAU_4 / 255.f) * 255.f);
}

void set_pixel(unsigned char *texture, unsigned offset, unsigned size, unsigned x, unsigned y, unsigned char val)
{
    texture[offset + x + y * size] = val % 0xff == 0 ? val : fine(val);
}

using font_face_t = std::unique_ptr<std::remove_pointer_t<FT_Face>, decltype(&FT_Done_Face)>;

struct glyph_view
{
    FT_Face face;

    struct glyph_iterator
    {
        FT_Face face;
        FT_UInt gindex;
        FT_ULong code;

        std::pair<FT_ULong, FT_UInt> operator*() const
        {
            return { code, gindex };
        }

        glyph_iterator& operator++()
        {
            code = FT_Get_Next_Char(face, code, &gindex);
            return *this;
        }
    };

    glyph_iterator begin() const
    {
        glyph_iterator out;
        out.face = face;
        out.code = FT_Get_First_Char(face, &out.gindex);
        return out;
    }

    struct sentinel
    {
        friend bool operator!=(const sentinel&, const glyph_iterator& g)
        {
            return !!g.gindex;
        }

        friend bool operator!=(const glyph_iterator& g, const sentinel&)
        {
            return !!g.gindex;
        }
    };

    sentinel end() const
    {
        return {};
    }
};

std::optional<ft_data_t> create_font(font_face_t freetype_font_face, const unsigned resolution, const unsigned cell_margin)
{
    FT_Set_Pixel_Sizes(freetype_font_face.get(), resolution, resolution);

    const unsigned character_count = freetype_font_face->num_glyphs;
    const unsigned cell_size = resolution + cell_margin * 2;
    const unsigned character_row_width = static_cast<unsigned>(std::ceil(std::sqrt(character_count)));
    const unsigned texSize = character_row_width * cell_size;
    const unsigned actual_texture_size = next_power_of_2(texSize);

    auto texture_data = std::make_unique<unsigned char[]>(actual_texture_size * actual_texture_size);

    glyph_map_t glyphs;

    math::point2<int> texture_position{0, 0};

    for (const auto [charcode, gindex] : glyph_view{freetype_font_face.get()})
    {
        if (FT_Load_Glyph(freetype_font_face.get(), gindex, FT_LOAD_DEFAULT)) continue;

        FT_GlyphSlot glyph = freetype_font_face->glyph;
        glyph_t g;
        FT_Render_Glyph(glyph, FT_RENDER_MODE_NORMAL);

        // Calculate glyph informations

        g.offset = { glyph->bitmap_left / static_cast<float>(cell_size),
                -(glyph->bitmap_top / static_cast<float>(cell_size)) };

        g.texture_position = { texture_position.x * cell_size / static_cast<float>(actual_texture_size),
                (texture_position.y * cell_size + 1) / static_cast<float>(actual_texture_size) };

        g.width = glyph->advance.x / static_cast<float>(64 * cell_size);

        glyphs.emplace(charcode, std::move(g));

        const unsigned base_offset = texture_position.x * cell_size + texture_position.y * cell_size * actual_texture_size;
        const unsigned gr = glyph->bitmap.rows;
        const unsigned gw = glyph->bitmap.width;

        for (unsigned h = 0; h < gr; ++h)
            for (unsigned w = 0; w < gw; ++w)
                set_pixel(texture_data.get(), base_offset + cell_margin, actual_texture_size, w, cell_margin + h,
                        glyph->bitmap.buffer[w + h * gw]);

        for (unsigned h = 0; h < cell_size; ++h)
        {
            if (h < cell_margin || h >= cell_margin + gr)
            {
                // bottom padding => whole rows
                for (unsigned w = 0; w < cell_size; ++w)
                {
                    set_pixel(texture_data.get(), base_offset, actual_texture_size, w, h, 0);
                }
            }
            else
            {
                // left padding
                for (unsigned w = 0; w < cell_margin; ++w)
                    set_pixel(texture_data.get(), base_offset, actual_texture_size, w, h, 0);
                // right padding
                for (unsigned w = gw + cell_margin; w < cell_size; ++w)
                    set_pixel(texture_data.get(), base_offset, actual_texture_size, w, h, 0);
            }
        }

        if (static_cast<unsigned>(++texture_position.x) >= character_row_width)
        {
            texture_position.x = 0;
            ++texture_position.y;
        }
    }

    return {{
        std::move(texture_data),
        static_cast<unsigned>(actual_texture_size),
        std::move(glyphs),
        cell_size / (float)actual_texture_size
    }};
}

FT_Library library;
unsigned library_ref_count = 0;
bool library_loaded = false;

}  // namespace


std::optional<ft_data_t> freetype_glue::operator()(const std::string_view &memory, const unsigned resolution) const
{
    if (!library_loaded) return {};

    if (FT_Face ff = nullptr;
            !FT_New_Memory_Face(
                library,
                reinterpret_cast<const FT_Byte *>(memory.data()),
                static_cast<FT_Long>(memory.size()), 0, &ff))
    {
        const auto cell_margin = static_cast<unsigned int>(std::ceil(static_cast<float>(resolution) / 10));
        return create_font({ ff, FT_Done_Face }, resolution, cell_margin);
    }

    LOGE("Error loading font face");
    return {};
}


freetype_glue::freetype_glue()
{
    if (++library_ref_count == 1)
    {
        library_loaded = FT_Init_FreeType(&library) == 0;

        if (!library_loaded)
        {
            LOGE("Failed to initialize freetype library");
        }
    }
}

freetype_glue::~freetype_glue()
{
    if (--library_ref_count == 0)
    {
        FT_Done_FreeType(library);
    }
}

}  // namespace fonts
