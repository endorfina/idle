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

#include <idle/drawable.hpp>
#include "room_model.hpp"

#ifdef IDLE_COMPILE_GALLERY

#include <idle/glass.hpp>

namespace idle::hotel::model
{

namespace
{

constexpr point_t flatten(const point_3d_t p)
{
    return { p.y, - p.z };
}

constexpr auto make_matrix(const point_3d_t angle)
{
    return math::matrices::rotate_x<float>(angle.x)
        * math::matrices::rotate_y<float>(angle.y);
}

constexpr std::array<point_t, 4> legs_to_points(const glass::blocks::arm& arm)
{
    std::array<point_3d_t, 4> pts{};
    auto rot = math::matrices::translate<float>({0, 0, arm.bones.root.length}) * make_matrix(arm.bones.root.angle);
    pts[1] = rot * point_3d_t{};

    rot = math::matrices::translate<float>({0, 0, arm.bones.get<0>().root.length}) * make_matrix(arm.bones.get<0>().root.angle) * rot;
    pts[2] = rot * point_3d_t{};

    rot = math::matrices::translate<float>({0, 0, arm.bones.get<0>().get<0>().length}) * make_matrix(arm.bones.get<0>().get<0>().angle) * rot;
    pts[3] = rot * point_3d_t{};

    std::array<point_t, 4> out{};
    for (unsigned i = 0; i < out.size(); ++i)
        out[i] = flatten(pts[i]);
    return out;
}

void draw_bones(const graphics::core& gl)
{
    constexpr auto hu = glass::closet::humanoid::get_default();
    constexpr auto pts = legs_to_points(hu.arms.left);
    gl.prog.fill.use();
    gl.prog.fill.set_view_transform(math::matrices::translate(math::point_cast<float>(gl.draw_size) / 2.f));
    gl.prog.fill.position_vertex(reinterpret_cast<const GLfloat*>(pts.data()));
    gl::DrawArrays(gl::LINE_STRIP, 0, pts.size());
}

}  // namespace

void room::draw(const graphics::core& gl) const
{
    gl.prog.fill.use();
    gl.prog.fill.set_identity();
    gl.prog.fill.set_view_identity();
    gl.prog.fill.set_color({0,0,0});
    fill_screen(gl, gl.prog.fill);

    gl.prog.text.use();
    gl.prog.text.set_color(color_t::greyscale(.29f));
    draw_text<text_align::center, text_align::center>(*gl.fonts.title, gl.prog.text, "Model", math::point_cast<float>(gl.draw_size) / 2.f, 48);

    gl.prog.fill.use();
    gl.prog.fill.set_color({1,0,0});
    draw_bones(gl);

}

}  // namespace idle::hotel::model

#endif  // IDLE_COMPILE_GALLERY

