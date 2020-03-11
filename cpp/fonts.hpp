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

#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include "gl_programs.hpp"

struct font_t
{
    struct glyph_t
    {
        idle::point_t offset, texture_position;
        float width;

    private:
        friend struct font_t;
        void draw(const graphics::text_program_t& rcp, float size, idle::point_t pos) const;
    };

    using map_t = std::unordered_map<unsigned long, glyph_t>;

    void draw(const graphics::text_program_t& rcp, const std::string_view &str, unsigned int limit = (-1)) const;

    void draw_custom_animation(const graphics::text_program_t& rcp, const std::string_view &str, const math::color<float> &col, const idle::text_animation_data* anim, unsigned start, unsigned end) const;


    //Return the width/height of the rendered text at given size
    idle::point_t get_extent(const std::string_view &str, float size, unsigned int limit = (-1)) const;

    std::string prepare_string(const std::string_view &str, float size, float max_width) const;

    font_t(GLuint texture, map_t character_map, float cell_size);

private:
    graphics::unique_texture texture;
    std::unordered_map<unsigned long, glyph_t> character_map;
    float cell_size, topmost_margin;
};
