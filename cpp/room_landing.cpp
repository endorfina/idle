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
#include <random>
#include "room_landing.hpp"
#include "gl.hpp"
#include "drawable.hpp"
#include "draw_text.hpp"

namespace idle
{
// #ifdef COMPILE_M_ROOM
// static void _draw_maker_room_button_(const graphics::core& gl, point_t pos, const gui_elem& elem)
// {
//     const float v[] = {
//             pos.x, pos.y,
//             pos.x + elem.bound.x, pos.y,
//             pos.x, pos.y + elem.bound.y
//     };
//     gl.prog.fill.use();
//     gl.prog.fill.set_color(elem.bg, elem.hover ? elem.bg.a : elem.bg.a / 2);
//     gl.prog.fill.position_vertex(v);
//     gl::DrawArrays(gl::TRIANGLE_STRIP, 0, 3);
//     gl.ptext.use();
//     gl.ptext.set_color(elem.fg);
//     gl.draw_text("Make", pos + elem.bound / 3.6f, 24, TextAlign::Center, TextAlign::Center);
// }
// #endif


landing_room::landing_room(graphics::core&)
{
// #ifdef COMPILE_M_ROOM
//     create_gui_elem(parent.collection, {0, 0}, {100, 50}, gui_elem::trigger_t::MakerRoom, _draw_maker_room_button_, false, {1.f, 0.f, .1f});
// #endif

    ray_array.fill(1.f);

    for (unsigned i = 0; i < ray_array.size(); ++i)
    {
        ray_array[i] += i % 2 == 0 ? -.1f : .1f;
    }

    ray_array_mirror.fill(1.f);
}

bool landing_room::step(graphics::core&)
{
    if (alpha < .998f)
    {
        alpha += .0008f;
    }
    else
    {
        alpha = 1.f;
    }

    rotation += .01f / APPLICATION_FPS;
    if (rotation > F_TAU)
        rotation -= F_TAU;

    if (!--counter)
    {
        std::minstd_rand gen{std::random_device{}()};
        std::uniform_real_distribution<float> dist_float{-.06f, .06f};

        for (unsigned i = 0; i < ray_array.size() * 4; ++i)
        {
            const auto j = i % ray_array.size();
            ray_array[j] = (ray_array[!i ? ray_array.size() - 1 : (i-1) % ray_array.size()] + ray_array[i % ray_array.size()] + ray_array[(i+1) % ray_array.size()] + dist_float(gen)) / 3;

            if (std::abs(1.f - ray_array[j]) > .3f)
                ray_array[j] = 0.f;
        }

        std::uniform_int_distribution<int> dist_int{APPLICATION_FPS * 4, APPLICATION_FPS * 6};
        counter = dist_int(gen);
    }

    for (unsigned i = 0; i < ray_array.size(); ++i)
    {
        ray_array_mirror[i] += (ray_array[i] - ray_array_mirror[i]) * .4f / APPLICATION_FPS;
    }

    // if (auto clicked_obj = check_hover_and_get_clicked(parent))
    //     switch (std::get<gui_elem>(clicked_obj->data).trigger)
    //     {
    //         case gui_elem::trigger_t::MakerRoom:
    //             parent.room_next_id = room_id_enum::room_models;
    //             break;
    //         default:
    //             break;
    //     }
    return true;
}

void landing_room::draw(const graphics::core& gl) const
{
    const auto alpha_sine = std::sin(alpha * F_TAU_4);
    const auto alpha_sine_sqr = alpha_sine * alpha_sine;
    gl.prog.fill.use();
    gl.prog.fill.set_color(1.f - .64f * alpha_sine, 1.f - .9f * alpha_sine, 1.f - .8f * alpha_sine, alpha_sine);
    gl.prog.fill.set_identity();
    gl.prog.fill.set_view_identity();

    fill_rectangle(gl.prog.fill, { float(gl.draw_size.x), float(gl.draw_size.y) });

    gl.prog.normal.use();
    gl.prog.normal.set_color(.021f, 0.f, .089f, alpha_sine);
    gl.prog.normal.set_view_transform(mat4x4_t::translate(gl.draw_size.x / 2, gl.draw_size.y / 2));

    constexpr auto div = F_TAU / static_cast<float>(std::tuple_size<TYPE_REMOVE_CVR(ray_array)>::value);
    constexpr auto array_len = std::tuple_size<TYPE_REMOVE_CVR(ray_array)>::value + 2;
    float fi = 0.f;

    std::array<point_t, array_len> points{};
    std::transform(ray_array_mirror.begin(), ray_array_mirror.end(), points.begin() + 1,
            [&](const auto f) -> point_t
            {
                fi += div;
                return { std::cos(fi) * f, -std::sin(fi) * f };
            });

    points[array_len - 1] = points[1];

    constexpr std::array<point_t, array_len> tex_map = []()
    {
        std::array<point_t, array_len> out{};
        for (unsigned i = 1; i < array_len; ++i)
            out[i] = point_t{0, 1.1f};
        return out;
    }();

    gl.prog.normal.set_transform(mat4x4_t::rotate(rotation) * mat4x4_t::scale(gl.draw_size.y * (1.1f - alpha_sine / 2)));

    gl::ActiveTexture(gl::TEXTURE0);
    gl::BindTexture(gl::TEXTURE_2D, gl.image_id_fade);
    gl.prog.normal.position_vertex(reinterpret_cast<const float *>(&points[0]));
    gl.prog.normal.texture_vertex(reinterpret_cast<const float *>(tex_map.data()));
    gl::DrawArrays(gl::TRIANGLE_FAN, 0, array_len);

    constexpr std::array<point_t, array_len> tex_map2 = []()
    {
        std::array<point_t, array_len> out{};
        out[0] = point_t{0, 1.021f};
        return out;
    }();

    gl.prog.normal.set_color(.9f, .1f, .2f, alpha_sine);
    gl.prog.normal.set_transform(mat4x4_t::rotate(rotation) * mat4x4_t::scale(gl.draw_size.y * (.3f + .55f * alpha_sine)));
    gl.prog.normal.texture_vertex(reinterpret_cast<const float *>(tex_map2.data()));
    gl::DrawArrays(gl::TRIANGLE_FAN, 0, array_len);
}

}  // namespace idle

