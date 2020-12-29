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

#include <random>
#include <cmath>
#include <algorithm>
#include <utility>
#include <idle/drawable.hpp>
#include "room_stage.hpp"
#include "../draw_text.hpp"

#include <idle/glass/glass.hpp>

namespace idle::hotel::stage
{

room::room() noexcept
    : player{ &objs.emplace(point_t{150.f,0}) }

    // action_vec([this] (auto& var)
    //     {
    //         using T = idle_remove_cvr(var);

    //         if constexpr(std::is_same_v<actions::step, T>)
    //         {
    //             if (const auto a = var.it->step(); a != action::none)
    //             {
    //                 const std::lock_guard lock{ action_vec.mutex };
    //                 switch(var.it->step())
    //                 {
    //                     case action::destroy:
    //                         action_vec.destroy.push_back(var.it);

    //                     case action::unregister_drawable:
    //                         action_vec.hide.push_back(&*var.it);
    //                         break;

    //                     case action::register_drawable:
    //                         action_vec.show.push_back(&*var.it);
    //                         break;

    //                     default:
    //                         break;
    //                 }
    //             }
    //         }
    //     })
{
    pictures.load_image("octavia_tex.png", std::get<crimson::characters::octavia>(player.captive_mind->variant).tex, gl::NEAREST);
    player.camera.translate = { 200, 200 };

    std::minstd_rand gen{};
    std::uniform_int_distribution<unsigned> rando{ 0, 255 };
    std::generate(floor_tiles.begin(), floor_tiles.end(),
            [&](){ return static_cast<uint8_t>(rando(gen)); });
}

void room::draw(const graphics::core& gl) noexcept
{
    gl.prog.fill.use();
    gl.prog.fill.set_identity();
    gl.prog.fill.set_view_identity();
    gl.view_mask();
    gl.prog.fill.set_color(graphics::black, .125f);
    fill_screen(gl, gl.prog.fill);

    gl.view_normal();
    gl.prog.fill.set_color(graphics::black);
    fill_screen(gl, gl.prog.fill);

    gl.prog.fill.set_color({1,1,1});

    constexpr mat4x4_noopt_t skew_matrix =
        math::matrices::scale(point_3d_t{ 1, 1 / .61f, 1 })
        * math::matrices::rotate(-math::tau_8);

    constexpr mat4x4_noopt_t anti_skew_matrix =
        math::matrices::rotate(math::tau_8)
        * math::matrices::scale(point_3d_t{1, .61f, 0});

    const auto view_mat = [this, &anti_skew_matrix, &gl]()
    {
        auto mat = anti_skew_matrix;
        math::transform::translate(mat, point_t{0, 30});
        math::transform::uniform_scale(mat, player.camera.scale);
        math::transform::translate(mat, gl.draw_size / 2);
        return mat;
    }();

    const auto model_mat = [this, &skew_matrix]()
        {
            auto mat = skew_matrix;
            math::transform::translate(mat, player.camera.translate * -1.f);
            return mat;
        }();

    static constexpr std::array<point_t, 4> tile_rectangle
    {
        point_t{0, 0},
        point_t{32, 0},
        point_t{0, 32},
        point_t{32, 32}
    };

    gl.prog.fill.position_vertex(reinterpret_cast<const GLfloat*>(&tile_rectangle[0]));
    gl.prog.fill.set_view_transform(view_mat);

    for (int y = 0; y < 32; ++y)
        for (int x = 0; x < 32; ++x)
        {
            if (x == int(player.captive_mind->pos.x / 32) && y == int(player.captive_mind->pos.y / 32))
            {
                gl.prog.fill.set_color(color_t::greyscale(1.f));
            }
            else
            {
                gl.prog.fill.set_color(color_t::greyscale(.2f + floor_tiles[y * 32 + x] / 600.f));
            }

            gl.prog.fill.set_transform(math::matrices::translate(point_t{ x * 32.f, y * 32.f } - player.camera.translate));
            gl::DrawArrays(gl::TRIANGLE_STRIP, 0, 4);
        }

    gl.prog.fill.set_transform(model_mat);
    gl.prog.fill.set_view_transform(view_mat);

    constexpr std::array<std::array<point_t, 2>, 2> helper_lines
    {
        std::array
        {
            point_t{ },
            point_t{ 150.f, 0.f }
        },
        std::array
        {
            point_t{ },
            point_t{ 0.f, 150.f }
        }
    };

    gl.prog.fill.position_vertex(reinterpret_cast<const GLfloat*>(&helper_lines[0][0]));
    gl::DrawArrays(gl::LINES, 0, helper_lines.size() * 2);

    gl.prog.double_normal.use();
    gl.prog.double_normal.set_color({1,1,1});
    gl.prog.double_normal.set_transform(skew_matrix);
    gl.prog.double_normal.set_view_transform(view_mat);

    const std::lock_guard block_object_destruction{ cell_mod_mutex };
    for (const auto it : render_order[draw_fork.load(std::memory_order_acquire)])
    {
        auto mat = model_mat;
        math::transform::translate(mat, it->pos);
        gl.prog.double_normal.set_transform(mat);
        it->draw(gl);
    }

    gl.prog.text.use();
    gl.prog.text.set_color({1,1,1});
    char str[120];
    std::snprintf(str, 120, "camera: [%.1f, %.1f]\ncursor: [%.1f, %.1f]",
            player.camera.translate.x, player.camera.translate.y,
            player.cursor_pos.x, player.cursor_pos.y);
    draw_text<text_align::near, text_align::near>(*gl.fonts.regular, gl.prog.text, str, point_t{10, 50}, 16);
}

void room::on_resize(const point_t size) noexcept
{
    player.hud_size = size;
}

std::optional<keyring::variant> room::step(const pointer_wrapper& pointer) noexcept
{
    std::vector<iterator_type> action_destroy;
    std::vector<const object*> action_hide, action_show;

    const auto end_iter = objs.end();
    for (auto it = objs.begin(); it != end_iter; ++it)
    {
        switch(it->step())
        {
            case action::destroy:
                action_destroy.push_back(it);

            case action::unregister_drawable:
                action_hide.push_back(&*it);
                break;

            case action::register_drawable:
                action_show.push_back(&*it);
                break;

            default:
                break;
        }
    }

    player.camera.scale = 1.5f;

    if (player.captive_mind)
    {
        player.cursor_pos = pointer.cursor.pos;
        player.camera.translate += (player.captive_mind->pos - player.camera.translate) * (.08f * uni_time_factor);

        const auto center = player.hud_size / 2;

        if (pointer.touch > .3f && center.distance(pointer.cursor.pos) > 30.f)
        {
            constexpr auto a = math::matrices::rotate(math::tau_8 * -2.5f) * point_t{1.f, 0.f};
            const auto vec = pointer.cursor.pos - center;
            const auto b = vec * -1.f / std::hypotf(vec.x, vec.y);
            const auto prod = a.product(b);
            const auto det = a.determinant(b);
            const float val = std::atan2(det, prod);
            player.captive_mind->move(val, .5f);
        }
    }

    const auto current_draw_fork = draw_fork.load(std::memory_order_relaxed);
    const auto next_draw_fork = (current_draw_fork + 1) % render_order.size();
    const auto& current_draw_order = render_order[current_draw_fork];
    auto& next_draw_order = render_order[next_draw_fork];

    next_draw_order.resize(current_draw_order.size());
    std::copy(current_draw_order.begin(), current_draw_order.end(), next_draw_order.begin());

    if (!!action_hide.size())
    {
        next_draw_order.erase(std::remove_if(next_draw_order.begin(), next_draw_order.end(),
                [&hide = action_hide](const auto ptr)
                {
                    return std::any_of(hide.begin(), hide.end(), [ptr](const auto hidden)
                            {
                                return hidden == ptr;
                            });
                }), next_draw_order.end());
    }

    if (!!action_show.size())
    {
        next_draw_order.insert(next_draw_order.end(), action_show.begin(), action_show.end());
    }

    std::sort(next_draw_order.begin(), next_draw_order.end(),
            [](const auto lhs, const auto rhs)
            {
                return lhs->pos.x + lhs->pos.y < rhs->pos.x + rhs->pos.y;
            });

    if (!!action_destroy.size())
    {
        const std::lock_guard block_drawing{ cell_mod_mutex };
        for (auto it : action_destroy)
        {
            if (player.captive_mind == &*it)
            {
                player.captive_mind = nullptr;
            }
            objs.remove(it);
        }
    }

    draw_fork.store(next_draw_fork, std::memory_order_release);

    return {};
}

void room::kill_workers() noexcept
{
    garment::loader::kill_workers();
}


}  // namespace idle::hotel::stage

