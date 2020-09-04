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
#include <vector>

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
using font_face_t = std::unique_ptr<std::remove_pointer_t<FT_Face>, decltype(&FT_Done_Face)>;

struct glyph_view
{
    FT_Face face;

    struct position
    {
        FT_UInt gindex;
        FT_ULong code;
    };

    struct iterator
    {
        FT_Face face;
        position pos;

        position operator*() const noexcept
        {
            return pos;
        }

        iterator& operator++() noexcept
        {
            pos.code = FT_Get_Next_Char(face, pos.code, &pos.gindex);
            return *this;
        }
    };

    iterator begin() const noexcept
    {
        iterator out;
        out.face = face;
        out.pos.code = FT_Get_First_Char(face, &out.pos.gindex);
        return out;
    }

    struct sentinel
    {
        friend bool operator!=(const iterator& it, const sentinel&) noexcept
        {
            return !!it.pos.gindex;
        }
    };

    sentinel end() const noexcept
    {
        return {};
    }
};



std::optional<ft_data_t> create_font(const font_face_t freetype_font_face, const unsigned resolution, bool (* filter_function)(unsigned long)) noexcept
{
    const auto filtered_chars = [&filter_function, ft = freetype_font_face.get()]()
    {
        std::vector<glyph_view::position> out;
        for (const auto it : glyph_view{ ft })
        {
            if (filter_function(it.code))
                out.push_back(it);
        }
        return out;
    }();

    const unsigned character_row_width = static_cast<unsigned>(std::ceil(std::sqrt(filtered_chars.size())));
    const unsigned cell_size = resolution / character_row_width;
    const unsigned cell_margin = std::max(2u, cell_size / 13);
    const unsigned glyph_apparent_height = cell_size - cell_margin * 2;

    LOGD("Given %zu chars and %upx resolution, calculated %u columns of %upx cells (%upx w/o margins)",
            filtered_chars.size(),
            resolution,
            character_row_width,
            cell_size,
            glyph_apparent_height);

    FT_Set_Pixel_Sizes(freetype_font_face.get(), glyph_apparent_height, glyph_apparent_height);

    auto texture_data = std::make_unique<unsigned char[]>(resolution * resolution);
    ::memset(texture_data.get(), 0, resolution * resolution);

    glyph_map_t glyphs;

    math::point2<unsigned int> texture_position{0, 0};

    for (const auto [gindex, charcode] : filtered_chars)
    {
        if (!!FT_Load_Glyph(freetype_font_face.get(), gindex, FT_LOAD_DEFAULT))
        {
            LOGW("Failed to load glyph 0x%03lx", charcode);
            continue;
        }

        FT_GlyphSlot glyph = freetype_font_face->glyph;
        FT_Render_Glyph(glyph, FT_RENDER_MODE_NORMAL);

        glyphs.emplace(charcode, glyph_t{
            { glyph->bitmap_left / static_cast<float>(cell_size),
                - glyph->bitmap_top / static_cast<float>(cell_size) }, // offset

            { texture_position.x * cell_size / static_cast<float>(resolution),
                (texture_position.y * cell_size + 1) / static_cast<float>(resolution) }, // texture_position

            glyph->advance.x / static_cast<float>(64 * cell_size)}); // width

        const unsigned base_offset = cell_size * (texture_position.x + texture_position.y * resolution);
        const unsigned gr = glyph->bitmap.rows;
        const unsigned gw = glyph->bitmap.width;
        unsigned char * const data_ptr = texture_data.get() + base_offset + cell_margin * (resolution + 1);

        for (unsigned y = 0; y < gr; ++y)
            for (unsigned x = 0; x < gw; ++x)
            {
                data_ptr[resolution * y + x] = glyph->bitmap.buffer[x + y * gw];
            }

        if (++texture_position.x >= character_row_width)
        {
            texture_position.x = 0;
            ++texture_position.y;
        }
    }

    return {{
        std::move(texture_data),
        static_cast<unsigned>(resolution),
        std::move(glyphs),
        cell_size / (float)resolution
    }};
}

constinit FT_Library library;
constinit unsigned library_ref_count = 0;
constinit bool library_loaded = false;

}  // namespace


std::optional<ft_data_t> freetype_glue::operator()(bool (* filter_function)(unsigned long), const std::string_view &memory, const texture_quality resolution) const noexcept
{
    if (!library_loaded) return {};

    if (FT_Face ff = nullptr;
            !FT_New_Memory_Face(
                library,
                reinterpret_cast<const FT_Byte *>(memory.data()),
                static_cast<FT_Long>(memory.size()), 0, &ff))
    {
        return create_font({ ff, FT_Done_Face }, static_cast<unsigned>(resolution), filter_function);
    }

    LOGE("Error loading font face");
    return {};
}


freetype_glue::freetype_glue() noexcept
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

freetype_glue::~freetype_glue() noexcept
{
    if (--library_ref_count == 0)
    {
        FT_Done_FreeType(library);
    }
}

}  // namespace fonts
