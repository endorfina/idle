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

#include <utf8.hpp>

#include "fonts.hpp"


font_t::font_t (GLuint atlasTex, font_t::map_t _glyphMap, float _glyphSize)
  : tex(atlasTex), character_map(std::move(_glyphMap)), cell_size(_glyphSize)
{
    for (auto &gi : character_map)
        if (gi.second.offset.y < topmost_margin)
            topmost_margin = gi.second.offset.y;
    topmost_margin *= .966f;
}

void font_t::glyph_t::draw(const graphics::text_program_t& rcp, float size, idle::point_t pos) const
{
    const float texCoords[8] = {
        texture_position.x, texture_position.y,
        texture_position.x + size, texture_position.y,
        texture_position.x, texture_position.y + size,
        texture_position.x + size, texture_position.y + size
    };
    pos += offset;
    rcp.set_text_offset(pos.x, pos.y);
    rcp.texture_vertex(texCoords);
    gl::DrawArrays(gl::TRIANGLE_STRIP, 0, 4);
}

#define LEFT_MARGIN 0.081f ///0.0415f

void font_t::draw(const graphics::text_program_t& rcp, const std::string_view &str, unsigned int limit) const
{
    if (limit == 0)
        return;
    gl::ActiveTexture(gl::TEXTURE0);
    gl::BindTexture(gl::TEXTURE_2D, tex);
    rcp.position_vertex(idle::square_coordinates);

    idle::point_t pos{-LEFT_MARGIN, -topmost_margin};

    for (const auto u8c : utf8x::translator<char>{str})
    {
        if (u8c == '\n') {
            pos.x = -LEFT_MARGIN;
            pos.y += 1;
        }
        else if (u8c > 0xd)
        {
            if (auto gi = character_map.find(u8c); gi != character_map.end()) {
                gi->second.draw(rcp, cell_size, pos);
                pos.x += gi->second.width;
            }
        }
        if (--limit < 1)
            break;
    }
}

void font_t::draw_custom_animation(const graphics::text_program_t& rcp, const std::string_view &str, const ::violet::color<float> &col, const idle::text_animation_data* anim, const unsigned start, const unsigned end) const
{
    gl::ActiveTexture(gl::TEXTURE0);
    gl::BindTexture(gl::TEXTURE_2D, tex);
    rcp.position_vertex(idle::square_coordinates);

    idle::point_t pos{-LEFT_MARGIN, -topmost_margin};

    rcp.set_color(col);
    unsigned int i = 0;
    anim += start;

    for (const auto u8c : utf8x::translator<char>{str})
    {
        if (u8c == '\n') {
            pos.x = -LEFT_MARGIN;
            pos.y += 1;
        }
        else if (u8c > 0xd)
        {
            if (auto gi = character_map.find(u8c); gi != character_map.end()) {
                if (i >= start) {
                    if (anim[1].scale < F_TAU_4)
                        rcp.set_color(col, col.a * (1 - ::violet::sqr(cosf(anim->scale))));
                    rcp.set_transform(idle::mat4x4_t::scale(1 - cosf(anim->scale) / 2) * idle::mat4x4_t::rotate(anim->rotation));
                    gi->second.draw(rcp, cell_size, pos);
                    ++anim;
                }
                else gi->second.draw(rcp, cell_size, pos);
                pos.x += gi->second.width;
            }
        }
        if (++i > end)
            break;
    }
    rcp.set_identity();
}

::idle::point_t font_t::get_extent(const std::string_view& str, float size, unsigned int limit) const
{
    float max_line_width = 0;
    int line_amount = 1;
    float current_line_width = 0;

    if (limit == 0)
        return { 0, 0 };

    for (const auto u8c : utf8x::translator<char>{str})
    {
        if (u8c == '\n') {
            ++line_amount;
            if (current_line_width > max_line_width)
                max_line_width = current_line_width;
            current_line_width = 0;
        }
        else if (u8c > 0xd)
        {
            if (auto gi = character_map.find(u8c); gi != character_map.end())
                current_line_width += gi->second.width * size;
        }
        if (--limit < 1)
            break;
    }
    if (current_line_width > max_line_width)
        max_line_width = current_line_width;

    return { max_line_width, line_amount * size };
}


std::string font_t::prepare_string(const std::string_view &str, const float size, const float width) const
{
    std::string out;
    float current_line_width = 0;
    size_t last_space = 0, write_pos = 0;

    for (utf8x::translator<char> ut(str); !ut.is_at_end(); ++ut)
    {
        if (const auto u8c = *ut; u8c == '\n') {
            current_line_width = 0;
            if (write_pos++ == out.size())
                out += '\n';
        }
        else if (u8c > 0xd)
            if (auto gi = character_map.find(u8c); gi != character_map.end())
            {
                if(u8c == 0x20) {
                    const auto current_space = write_pos;
                    if (write_pos++ == out.size())
                        out += char(0x20);
                    if (width < current_line_width + gi->second.width * size) {
                        if (out[last_space] == 0x20) {
                            out[last_space] = '\n';
                            write_pos = last_space + 1;
                            ut = str.substr(write_pos);
                            last_space = current_space;
                            current_line_width = 0;
                        }
                        else last_space = current_space;
                        continue;
                    }
                    last_space = current_space;
                }
                else {
                    const unsigned clen = utf8x::code_length(u8c);
                    if (write_pos == out.size()) {
                        out.resize(write_pos + clen);
                        utf8x::put_switch(&out[write_pos], clen, u8c);
                    }
                    write_pos += clen;
                }
                current_line_width += gi->second.width * size;
            }
    }
    if (width < current_line_width) {
        if (out[last_space] == 0x20)
            out[last_space] = '\n';
    }
    return out;
}
