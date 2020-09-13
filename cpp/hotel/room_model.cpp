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

#ifdef IDLE_COMPILE_GALLERY

#include <utility>
#include <idle/drawable.hpp>
#include "room_model.hpp"

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

    void draw(const graphics::core& gl) const noexcept
    {
        auto ptr = mesh.table.data();

        for (const auto it : mesh.lengths)
        {
            gl.prog.fill.position_vertex(reinterpret_cast<const GLfloat*>(ptr));
            gl::DrawArrays(gl::LINE_STRIP, 0, it);
            ptr += it;
        }
    }

    void draw_interpolated(const graphics::core& gl, const line_mesh& another) const noexcept
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

    explicit constexpr line_mesh(const tree_type& tree) noexcept : mesh{tree}
    {
    }
};

template<typename S, auto...Is>
constexpr auto skew_index(const S& source, const mat4x4_noopt_t& mat, std::index_sequence<Is...>) noexcept
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

constexpr mat4x4_noopt_t skew_matrix = math::matrices::rotate<GLfloat>(math::tau_8) * math::matrices::rotate_y<GLfloat>(math::tau_4 / 3);

template<unsigned...Deg, typename Rig, auto Size>
constexpr auto skew(const std::array<Rig, Size>& anim) noexcept
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

constexpr float walk_leg_raise = math::tau_8 / 6.f;
constexpr float walk_knee_bend = math::tau_8;
constexpr float walk_foot_raise = math::tau_8 * -.6f;
constexpr float walk_hip_swing = math::tau_8 / 12.f;

constexpr float walk_shoulder_swing = math::tau_8 / 16.f;
constexpr float walk_arm_raise = math::tau_8 / 8.f;
constexpr float walk_elbow_bend = math::tau_8 / 4.f;

constexpr auto walking_muscle_digest = glass::muscle
    {
        std::make_tuple(
            // [](glass::closet::humanoid h) { return h; },

            [](glass::closet::humanoid h)
            {
                h.get_ref<glass::closet::parts::upperbody>().root.angle.z -= walk_shoulder_swing;
                h.get_ref<glass::closet::parts::lowerbody>().root.angle.z += walk_hip_swing;

                auto& shoulders = h.get_ref<glass::closet::parts::shoulders>();
                shoulders.right[1].angle.x += math::tau_8 * .61f;
                shoulders.left[1].angle.x -= math::tau_8 * .61f;
                shoulders.right[0].angle.y += walk_arm_raise / 2;
                shoulders.right[2].angle.y += walk_elbow_bend;
                shoulders.left[0].angle.y -= walk_arm_raise / 2;
                shoulders.left[2].angle.y += walk_elbow_bend;

                auto& hips = h.get_ref<glass::closet::parts::hips>();
                hips.right[0].angle.y -= walk_leg_raise;
                hips.right[2].angle.y += walk_knee_bend;
                hips.right[3].angle.y += walk_leg_raise + walk_foot_raise;
                hips.left[0].angle.y += walk_leg_raise;
                hips.left[3].angle.y -= walk_leg_raise;

                h.realign();
                return h;
            },

            [](glass::closet::humanoid h)
            {
                h.get_ref<glass::closet::parts::upperbody>().root.angle.z -= walk_shoulder_swing;
                h.get_ref<glass::closet::parts::lowerbody>().root.angle.z += walk_hip_swing;

                auto& shoulders = h.get_ref<glass::closet::parts::shoulders>();
                shoulders.right[0].angle.y += walk_arm_raise;
                shoulders.left[0].angle.y -= walk_arm_raise;
                shoulders.left[2].angle.y += walk_elbow_bend;

                auto& hips = h.get_ref<glass::closet::parts::hips>();

                hips.right[0].angle.y -= walk_leg_raise;
                hips.right[2].angle.y -= walk_knee_bend;
                hips.right[3].angle.y += walk_leg_raise - walk_foot_raise;
                hips.left[0].angle.y += walk_leg_raise;
                hips.left[3].angle.y -= walk_leg_raise;

                h.realign();
                return h;
            },

            [](glass::closet::humanoid h)
            {
                h.get_ref<glass::closet::parts::upperbody>().root.angle.z += walk_shoulder_swing * 2;
                h.get_ref<glass::closet::parts::lowerbody>().root.angle.z -= walk_hip_swing * 2;

                auto& shoulders = h.get_ref<glass::closet::parts::shoulders>();
                shoulders.right[0].angle.y -= walk_arm_raise * 3;
                shoulders.left[0].angle.y += walk_arm_raise * 3;
                shoulders.left[2].angle.y -= walk_elbow_bend;

                auto& hips = h.get_ref<glass::closet::parts::hips>();

                hips.right[0].angle.y += walk_leg_raise * 3;
                hips.right[3].angle.y -= walk_leg_raise * 3;
                hips.left[0].angle.y -= walk_leg_raise * 3;
                hips.left[2].angle.y += walk_knee_bend;
                hips.left[3].angle.y += walk_leg_raise * 3 + walk_foot_raise;

                h.realign();
                return h;
            },

            [](glass::closet::humanoid h)
            {
                h.get_ref<glass::closet::parts::upperbody>().root.angle.z += walk_shoulder_swing;
                h.get_ref<glass::closet::parts::lowerbody>().root.angle.z -= walk_hip_swing;

                auto& shoulders = h.get_ref<glass::closet::parts::shoulders>();
                shoulders.right[0].angle.y -= walk_arm_raise;
                shoulders.left[0].angle.y += walk_arm_raise;
                shoulders.right[2].angle.y += walk_elbow_bend;

                auto& hips = h.get_ref<glass::closet::parts::hips>();

                hips.left[0].angle.y -= walk_leg_raise;
                hips.left[2].angle.y -= walk_knee_bend;
                hips.left[3].angle.y += walk_leg_raise - walk_foot_raise;
                hips.right[0].angle.y += walk_leg_raise;
                hips.right[3].angle.y -= walk_leg_raise;

                h.realign();
                return h;
            }
        )
    }.animate(hueman);

constexpr auto walking = skew<0, 45, 90, 135, 180, 225, 270, 315>(walking_muscle_digest);

constexpr auto floating_muscle_digest = glass::muscle
    {
        std::make_tuple(
            // [](glass::closet::humanoid h) { return h; },

            [](glass::closet::humanoid h)
            {
                auto& [neck, face] = h.get_ref<glass::closet::parts::head>().table;
                neck.angle.y -= math::tau_8 * .2f;
                h.get_ref<glass::closet::parts::upperbody>().root.angle.y -= math::tau_8 *.8f;
                h.get_ref<glass::closet::parts::lowerbody>().root.angle.y -= math::tau_8 *.7f;

                auto& shoulders = h.get_ref<glass::closet::parts::shoulders>();
                // shoulders.right[1].angle.x += math::tau_8 * .61f;
                // shoulders.left[1].angle.x -= math::tau_8 * .61f;
                shoulders.right[0].angle.y -= math::tau_8 * .1f;
                shoulders.right[2].angle.y += math::tau_8 * .1f;
                shoulders.right[2].angle.z -= math::tau_8 * .1f;
                shoulders.left[0].angle.y += math::tau_8 * .1f;
                shoulders.left[2].angle.y += math::tau_8 * .1f;
                shoulders.left[2].angle.z += math::tau_8 * .1f;

                auto& hips = h.get_ref<glass::closet::parts::hips>();
                hips.right[0].angle.y += math::tau_8 *.3f;
                hips.right[1].angle.x += math::tau_8 *.19f;
                hips.right[2].angle.y -= math::tau_8 *.1f;
                hips.right[3].angle.y += math::tau_8;
                hips.left[0].angle.y += math::tau_8 * .5f;
                hips.left[1].angle.x -= math::tau_8 *.19f;
                hips.left[2].angle.y += math::tau_8 * .5f;
                hips.left[3].angle.y += math::tau_8;

                return h;
            },

            [](glass::closet::humanoid h)
            {
                auto& [neck, face] = h.get_ref<glass::closet::parts::head>().table;
                neck.angle.y -= math::tau_8 * .001f;
                h.get_ref<glass::closet::parts::upperbody>().root.angle.y -= math::tau_8 *.05f;
                h.get_ref<glass::closet::parts::lowerbody>().root.angle.y -= math::tau_8 *.05f;

                auto& shoulders = h.get_ref<glass::closet::parts::shoulders>();
                // shoulders.right[1].angle.x += math::tau_8 * .61f;
                // shoulders.left[1].angle.x -= math::tau_8 * .61f;
                shoulders.right[0].angle.y += math::tau_8 * .01f;
                shoulders.right[2].angle.y += math::tau_8 * .4f;
                shoulders.right[2].angle.z -= math::tau_8 * .3f;
                shoulders.left[0].angle.y -= math::tau_8 * .01f;
                shoulders.left[2].angle.y += math::tau_8 * .4f;
                shoulders.left[2].angle.z += math::tau_8;

                auto& hips = h.get_ref<glass::closet::parts::hips>();
                hips.right[0].angle.y += math::tau_8 *.1f;
                hips.right[2].angle.y += math::tau_8 *.05f;
                hips.left[0].angle.y -= math::tau_8 * .2f;
                hips.left[2].angle.y -= math::tau_8 * .1f;

                return h;
            }
        )
    }.animate(hueman);

constexpr auto floating = skew<0, 45, 90, 135, 180, 225, 270, 315>(floating_muscle_digest);

template<typename Models>
void draw_bones(const Models& models, const graphics::core& gl, const animation anim) noexcept
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

void room::draw(const graphics::core& gl) const noexcept
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

    gl::LineWidth(2.f);
    draw_bones(walking[facing], gl, model_anim.load(std::memory_order_relaxed));
}

std::optional<keyring::variant> room::step(const pointer_wrapper& cursor) noexcept
{
    auto work_copy = model_anim.load(std::memory_order_relaxed);
    timer += .12f / math::tau_2 * uni_time_factor;

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

