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

#include <utility>
#include <idle/drawable.hpp>
#include "room_model.hpp"

#ifdef IDLE_COMPILE_GALLERY

#include <idle/glass.hpp>

namespace idle::hotel::model
{

namespace
{

template<typename...Js>
struct line_mesh
{
    using tree_type = glass::flat_tree<Js...>;

    tree_type mesh;

    void draw(const graphics::core& gl) const
    {
        auto ptr = mesh.table.data();

        for (const auto it : mesh.lengths)
        {
            gl.prog.fill.position_vertex(reinterpret_cast<const GLfloat*>(ptr));
            gl::DrawArrays(gl::LINE_STRIP, 0, it);
            ptr += it;
        }
    }

    void draw_interpolated(const graphics::core& gl, const line_mesh& another) const
    {
        size_t sum = 0;

        for (const auto it : mesh.lengths)
        {
            gl.prog.double_fill.position_vertex(reinterpret_cast<const GLfloat*>(&mesh.table[sum]));
            gl.prog.double_fill.destination_vertex(reinterpret_cast<const GLfloat*>(&another.mesh.table[sum]));
            gl::DrawArrays(gl::LINE_STRIP, 0, it);
            sum += it;
        }
    }

    explicit constexpr line_mesh(const tree_type& tree) : mesh{tree}
    {
    }
};

template<typename S, auto...Is>
constexpr auto skew_index(const S& source, const mat4x4_noopt_t& mat, std::index_sequence<Is...>)
{
    return std::array{
        line_mesh{
            glass::flat_tree(
                glass::deep_tree(
                    source[Is],
                    mat
                )) } ...
    };
}

constexpr mat4x4_noopt_t skew_matrix = math::matrices::rotate<GLfloat>(F_TAU_8) * math::matrices::rotate_y<GLfloat>(F_TAU_4 / 3);

template<unsigned...Deg, typename Rig, auto Size>
constexpr auto skew(const std::array<Rig, Size>& anim)
{
    auto fun = [&anim] (const float deg)
    {
        return skew_index(
                anim,
                math::matrices::rotate<float>(math::degtorad<float>(deg)) * skew_matrix,
                std::make_index_sequence<Size>{});
    };

    return std::array { fun(static_cast<float>(Deg)) ... };
}

constexpr glass::closet::humanoid hueman{};

constexpr float walk_leg_raise = F_TAU_8 / 4.f;
constexpr float walk_knee_bend = F_TAU_8;
constexpr float walk_hip_swing = F_TAU_8 / 8.f;

constexpr auto walking_muscle_digest = glass::muscle
    {
        std::make_tuple(
            // [](glass::closet::humanoid h) { return h; },

            [](glass::closet::humanoid h)
            {
                h.get_lowerbody().root.angle.z += walk_hip_swing;

                auto& hips = h.get_hips();
                hips.right[0].angle.y -= walk_leg_raise;
                hips.right[2].angle.y += walk_knee_bend;
                hips.right[3].angle.y -= walk_leg_raise * 2;
                hips.left[0].angle.y += walk_leg_raise;
                hips.left[3].angle.y -= walk_leg_raise;

                h.realign();
                return h;
            },

            [](glass::closet::humanoid h)
            {
                h.get_lowerbody().root.angle.z += walk_hip_swing;

                auto& hips = h.get_hips();

                hips.right[0].angle.y -= walk_leg_raise;
                hips.right[2].angle.y -= walk_knee_bend;
                hips.right[3].angle.y += walk_knee_bend;
                hips.left[0].angle.y += walk_leg_raise;
                hips.left[3].angle.y -= walk_leg_raise;

                h.realign();
                return h;
            },

            [](glass::closet::humanoid h)
            {
                h.get_lowerbody().root.angle.z -= walk_hip_swing * 3;

                auto& hips = h.get_hips();

                hips.right[0].angle.y += walk_leg_raise * 3;
                hips.right[3].angle.y += walk_leg_raise - walk_knee_bend;
                hips.left[0].angle.y -= walk_leg_raise * 3;
                hips.left[2].angle.y += walk_knee_bend;

                h.realign();
                return h;
            },

            [](glass::closet::humanoid h)
            {
                h.get_lowerbody().root.angle.z -= walk_hip_swing;

                auto& hips = h.get_hips();

                hips.left[0].angle.y -= walk_leg_raise;
                hips.left[2].angle.y -= walk_knee_bend;
                hips.left[3].angle.y += walk_knee_bend;
                hips.right[0].angle.y += walk_leg_raise;
                hips.right[3].angle.y -= walk_leg_raise;

                h.realign();
                return h;
            }
        )
    }.animate(hueman);

constexpr auto walking = skew<0, 45, 90, 135, 180, 225, 270, 315>(walking_muscle_digest);

template<typename Models>
void draw_bones(const Models& models, const graphics::core& gl, const animation anim)
{
    gl.prog.fill.use();
    gl.prog.fill.set_color({1,0,0});

    const auto model_offset = gl.draw_size.x / (models.size() + 1);
    gl.prog.fill.set_view_transform(math::matrices::translate<float>({model_offset, gl.draw_size.y * .8f}));

    unsigned i = 0;
    for (const auto& it : models)
    {
        gl.prog.fill.set_transform(math::matrices::translate<float>({model_offset * i++, 0}));
        it.draw(gl);
    }

    gl.prog.double_fill.use();
    gl.prog.double_fill.set_color({1,1,1,.9f});
    const auto view = math::matrices::uniform_scale(2.f) * math::matrices::translate(gl.draw_size / 2.f);
    gl.prog.double_fill.set_view_transform(view);
    gl.prog.double_fill.set_interpolation(anim.interpolation);

    models[anim.source % models.size()].draw_interpolated(gl, models[anim.dest % models.size()]);
}

}  // namespace

void room::draw(const graphics::core& gl) const
{
    gl.prog.fill.use();
    gl.prog.fill.set_identity();
    gl.prog.fill.set_view_identity();
    gl.prog.fill.set_color({0,0,0});
    fill_screen(gl, gl.prog.fill);

    constexpr auto greyscale = color_t::greyscale(.29f);

    gl.prog.text.use();
    gl.prog.text.set_color(greyscale);
    draw_text<text_align::center, text_align::center>(*gl.fonts.title, gl.prog.text, "Model", gl.draw_size / 2.f, 48);

    // draw_bones(def_models_rotated, gl, model_anim.load(std::memory_order_relaxed));
    draw_bones(walking[facing], gl, model_anim.load(std::memory_order_relaxed));

}

std::optional<keyring::variant> room::step(const pointer_wrapper& cursor)
{
    auto work_copy = model_anim.load(std::memory_order_relaxed);
    timer += .12f / F_TAU_2;

    if (timer >= 1.f)
    {
        timer = 0.f;

        work_copy.source = work_copy.dest;

        if (++work_copy.dest >= walking[0].size())
        {
            work_copy.dest = 0;
        }
    }

    work_copy.interpolation = timer;
    model_anim.store(work_copy, std::memory_order_relaxed);

    if (cursor.single_press)
    {
        if (static_cast<unsigned>(facing) + 1 >= walking.size())
        {
            facing = 0;
        }
        else
        {
            ++facing;
        }
    }

    return {};
}

}  // namespace idle::hotel::model

#endif  // IDLE_COMPILE_GALLERY

