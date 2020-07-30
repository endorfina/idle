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

#include <math.hpp>
#include "platform/opengl_core_adaptive.hpp"
#include "idle_try.hpp"

namespace idle
{
using color_t = math::color<GLfloat>;
using point_t = math::point2<GLfloat>;
using point_3d_t = math::point3<GLfloat>;
using mat4x4_t = math::matrix4x4<GLfloat, 0>;
using mat4x4_noopt_t = math::matrix4x4<GLfloat, 1>;
using rect_t = math::rect<GLfloat>;

enum class text_align { near, center, far };

constexpr unsigned application_frames_per_second = 60;

constexpr float square_coordinates[8]
{
    0, 0, 1, 0,
    0, 1, 1, 1
};

struct text_animation_data
{
    float scale, rotation;
};

}  // namespace idle

