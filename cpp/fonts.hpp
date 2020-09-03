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

#pragma once
#include <string>
#include <string_view>
#include <math.hpp>
#include "glyph.hpp"
#include "gl_programs.hpp"

namespace fonts
{

struct font_t
{
    void draw(const graphics::text_program_t& rcp, const std::string_view &str, unsigned int limit = (-1)) const noexcept;

    void draw_custom_animation(const graphics::text_program_t& rcp, const std::string_view &str, const math::color<float> &col, const idle::text_animation_data* anim, unsigned start, unsigned end) const noexcept;

    //Return the width/height of the rendered text at given size
    idle::point_t get_extent(const std::string_view &str, float size, unsigned int limit = (-1)) const noexcept;

    std::string prepare_string(const std::string_view &str, float size, float max_width) const noexcept;

#ifndef IDLE_COMPILE_FONT_DEBUG_SCREEN
private:
#endif
    graphics::unique_texture texture;
    glyph_map_t character_map;
    float cell_size, min_y, max_y;

public:
    template<typename A, typename B>
    font_t(A&& tex, B&& map, const float size, const float mny, const float mxy) noexcept
        : texture(std::forward<A>(tex))
        , character_map(std::forward<B>(map))
        , cell_size(size)
        , min_y(mny)
        , max_y(mxy)
    {
    }
};

}  // namespace fonts
