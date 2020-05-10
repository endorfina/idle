/*
    Copyright © 2020 endorfina <dev.endorfina@outlook.com>

    This file is part of the Idle.

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
#pragma once
#include <string_view>
#include "gl.hpp"

namespace idle
{

namespace detail
{
template <TextAlign H, TextAlign V>
point_t get_text_transform(const fonts::font_t& font, std::string_view str, point_t p, float a, const unsigned int limit)
{
    if constexpr (H == TextAlign::Near && V == TextAlign::Near)
        return p;
    else
    {
        point_t fs = font.get_extent(str, a, limit);
        if constexpr (H == TextAlign::Near)
            fs.x = p.x;
        else if constexpr (H == TextAlign::Center)
            fs.x = p.x - fs.x / 2.0f;
        else
            fs.x = p.x - fs.x;

        if constexpr (V == TextAlign::Near)
            fs.y = p.y;
        else if constexpr (V == TextAlign::Center)
            fs.y = p.y - fs.y / 2.0f;
        else
            fs.y = p.y - fs.y;

        return fs;
    }
}

}  // namespace detail

template <TextAlign H = TextAlign::Near, TextAlign V = TextAlign::Near>
void draw_text(const graphics::core& gl, const std::string_view& str, point_t p, float size, const unsigned int limit = static_cast<unsigned int>(-1))
{
    gl.prog.text.set_view_transform(mat4x4_t::scale(size) * mat4x4_t::translate(detail::get_text_transform<H, V>(*gl.font, str, p, size, limit)));
    gl.font->draw(gl.prog.text, str, limit);
}

// void draw_text_animation1(const graphics::core& gl, const std::string_view& msg, float x, float y, float size, const color_t &col, const idle::text_animation_data* anim, const unsigned start, const unsigned end)
// {
//     gl.prog.text.set_view_transform(mat4x4_t::scale(size) * mat4x4_t::translate(x, y));
//     gl.font->draw_custom_animation(gl.prog.text, msg, col, anim, start, end);
// }

}  // namespace idle
