﻿/*
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

#include <math.hpp>

#include "lodge.hpp"

namespace idle
{

void lodge::draw(const graphics::core& gl) const
{
    auto bgsz = background.get_size<float>();
    const float bgsc = std::max(static_cast<float>(gl.draw_size.y) / bgsz.y, static_cast<float>(gl.draw_size.x) / bgsz.x);
    constexpr rect_t astronaut{0, 0, 398, 432}, /* text_loading{0, 512 - 70, 360, 512},*/ text_cp{450, 0, 512, 512};

    gl.prog.normal.use();
    gl.prog.normal.set_color({1, 1, 1, alpha});
    gl.prog.normal.set_transform(mat4x4_t::translate((gl.draw_size.x - bgsz.x) * .5f, (gl.draw_size.y - bgsz.y) * .5f) * mat4x4_t::scale(bgsc, bgsc, {gl.draw_size.x / 2.f, gl.draw_size.y / 2.f}));
    background.draw(gl.prog.normal);

    if (alpha < 1.f)
    {
        gl.prog.normal.set_transform(mat4x4_t::scale(sinf(alpha * F_TAU_4) *.5f + .5f, point_t((astronaut.right - astronaut.left) / 2, (astronaut.bottom - astronaut.top) / 2))
            * mat4x4_t::scale(.6f) * mat4x4_t::translate(gl.draw_size.x / 2 - 80.f, 25.f));
        gl.prog.normal.set_color({1, 1, 1, alpha * alpha});
    }
    else
    {
        gl.prog.normal.set_transform(mat4x4_t::scale(.6f) * mat4x4_t::translate(gl.draw_size.x / 2 - 80.f, 25.f));
    }
    picture.draw(gl.prog.normal, astronaut);

    gl.prog.normal.set_color({1, 1, 1, 1});
    gl.prog.normal.set_identity();
    gl.prog.normal.set_view_identity();

    gl.prog.fill.use();
    gl.prog.fill.set_identity();
    gl.prog.fill.set_view_identity();

    gl.prog.normal.use();
    gl.prog.normal.set_color({1, 1, 1, 1});
    gl.prog.normal.set_transform(mat4x4_t::rotate(math::degtorad<float>(90)) * mat4x4_t::scale(.4f) * mat4x4_t::translate(gl.draw_size.x - 1, 1));
    picture.draw(gl.prog.normal, text_cp);
}

void lodge::tick()
{
    if (!load_status.load(std::memory_order_acquire))
    {
        alpha = std::min<float>(alpha + .0251f, 1.f);
    }
    else
    {
        alpha = std::max<float>(alpha - .0333f, 0.f);
    }
}

bool lodge::is_done() const
{
    return (load_status.load(std::memory_order_acquire) && alpha == 0.f);
}

}  // namespace idle

