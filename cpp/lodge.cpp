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

#include <math.hpp>

#include "lodge.hpp"
// #include "hsv.hpp"

namespace idle
{

#ifdef DEBUG
lodge::~lodge()
{
    LOGD("Closing the lodge after %u frames", frame_count);
}
#endif

void lodge::draw(const graphics::core& gl)
{
#ifdef DEBUG
    ++frame_count;
#endif

    auto bgsz = background.get_size<float>();
    const float bgsc = std::max(static_cast<float>(gl.draw_size.y) / bgsz.y, static_cast<float>(gl.draw_size.x) / bgsz.x);
    constexpr rect_t emily{0, 0, 398, 432}, /* text_loading{0, 512 - 70, 360, 512},*/ text_cp{450, 0, 512, 512};
    // const float hdiv = gl.draw_size.y / 6.f;
    // constexpr auto rbw = rainbow_from_saturation(.96f);

    gl.prog.normal.use();
    gl.prog.normal.set_color(1, 1, 1, 1);
    gl.prog.normal.set_transform(mat4x4_t::translate((gl.draw_size.x - bgsz.x) * .5f, (gl.draw_size.y - bgsz.y) * .5f) * mat4x4_t::scale(bgsc, bgsc, {gl.draw_size.x / 2.f, gl.draw_size.y / 2.f}));
    background.draw(gl.prog.normal);

    if (alpha < .9999f) {
        gl.prog.normal.set_transform(mat4x4_t::scale(sinf(alpha * F_TAU_4) *.5f + .5f, point_t((emily.right - emily.left) / 2, (emily.bottom - emily.top) / 2))
            * mat4x4_t::scale(.6f) * mat4x4_t::translate(gl.draw_size.x / 2 - 80.f, 25.f));
        gl.prog.normal.set_color(1, 1, 1, alpha * alpha);
        alpha += .025f;
    }
    else
    gl.prog.normal.set_transform(mat4x4_t::scale(.6f) * mat4x4_t::translate(gl.draw_size.x / 2 - 80.f, 25.f));
    picture.draw(gl.prog.normal, emily);

    gl.prog.normal.set_color(1, 1, 1, 1);
    gl.prog.normal.set_identity();
    gl.prog.normal.set_view_identity();

    gl.prog.fill.use();
    gl.prog.fill.set_identity();
    gl.prog.fill.set_view_identity();

    // for (int i = 0; i < 6; ++i) {
    //     gl.prog.fill.set_color(rbw[i], .51f);
    //     float ht = hdiv * i + iterator;
    //     if (ht >= static_cast<float>(gl.draw_size.y))
    //         ht -= static_cast<float>(gl.draw_size.y);
    //     else if (ht + hdiv >= static_cast<float>(gl.draw_size.y))
    //         fill_rectangle(gl.prog.fill, rect_t(0, ht - static_cast<float>(gl.draw_size.y), 30.f, ht + hdiv - static_cast<float>(gl.draw_size.y)));
    //     fill_rectangle(gl.prog.fill, rect_t(0, ht, 30.f, ht + hdiv));
    // }
    // if (++iterator >= static_cast<unsigned>(gl.draw_size.y))
    //     iterator = 0;

    gl.prog.normal.use();
    gl.prog.normal.set_color(1, 1, 1, 1);
    gl.prog.normal.set_transform(mat4x4_t::rotate(math::degtorad<float>(90)) * mat4x4_t::scale(.4f) * mat4x4_t::translate(gl.draw_size.x - 1, 1));
    picture.draw(gl.prog.normal, text_cp);
}

}  // namespace idle

