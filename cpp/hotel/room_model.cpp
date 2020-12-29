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

#include <idle/glass/glass.hpp>
#include <guard.hpp>
#include "../game/objects.hpp"

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
constexpr auto skew_lines_index(const S& source, const mat4x4_noopt_t& mat, std::index_sequence<Is...>) noexcept
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

template<unsigned...Deg, typename Rig, auto Size>
constexpr auto make_lines(const std::array<Rig, Size>& anim) noexcept
{
    const auto face = [&anim] (const float deg)
    {
        return skew_lines_index(
                anim,
                math::matrices::rotate<float>(math::degtorad<float>(deg)) * glass::meta::skew_matrix,
                std::make_index_sequence<Size>{});
    };

    return std::array { face(static_cast<float>(Deg)) ... };
}

constexpr auto walking_digest = crimson::characters::traits::humanoid::walking_muscle.animate(crimson::characters::traits::humanoid::default_model);
constexpr auto walking_lines = make_lines<0, 45, 90, 135, 180, 225, 270, 315>(walking_digest);
constexpr auto walking_paint = glass::make<0, 45, 90, 135, 180, 225, 270, 315>(glass::paint::human_mesh, walking_digest);
constexpr auto human_skin = glass::paint::human_mesh.texture();

// constexpr auto floating_muscle_digest = glass::muscle
//     {
//         std::make_tuple(
//             // [](glass::closet::humanoid h) { return h; },
//
//             [](glass::closet::humanoid h)
//             {
//                 auto& [neck, face] = h.get_ref<glass::parts::head>().table;
//                 neck.angle.y -= math::tau_8 * .2f;
//                 h.get_ref<glass::parts::upperbody>().root.angle.y -= math::tau_8 *.8f;
//                 h.get_ref<glass::parts::lowerbody>().root.angle.y -= math::tau_8 *.7f;
//
//                 auto& shoulders = h.get_ref<glass::parts::shoulders>();
//                 // shoulders.right[1].angle.x += math::tau_8 * .61f;
//                 // shoulders.left[1].angle.x -= math::tau_8 * .61f;
//                 shoulders.right[0].angle.y -= math::tau_8 * .1f;
//                 shoulders.right[2].angle.y += math::tau_8 * .1f;
//                 shoulders.right[2].angle.z -= math::tau_8 * .1f;
//                 shoulders.left[0].angle.y += math::tau_8 * .1f;
//                 shoulders.left[2].angle.y += math::tau_8 * .1f;
//                 shoulders.left[2].angle.z += math::tau_8 * .1f;
//
//                 auto& hips = h.get_ref<glass::parts::hips>();
//                 hips.right[0].angle.y += math::tau_8 *.3f;
//                 hips.right[1].angle.x += math::tau_8 *.19f;
//                 hips.right[2].angle.y -= math::tau_8 *.1f;
//                 hips.right[3].angle.y += math::tau_8;
//                 hips.left[0].angle.y += math::tau_8 * .5f;
//                 hips.left[1].angle.x -= math::tau_8 *.19f;
//                 hips.left[2].angle.y += math::tau_8 * .5f;
//                 hips.left[3].angle.y += math::tau_8;
//
//                 return h;
//             },
//
//             [](glass::closet::humanoid h)
//             {
//                 auto& [neck, face] = h.get_ref<glass::parts::head>().table;
//                 neck.angle.y -= math::tau_8 * .001f;
//                 h.get_ref<glass::parts::upperbody>().root.angle.y -= math::tau_8 *.05f;
//                 h.get_ref<glass::parts::lowerbody>().root.angle.y -= math::tau_8 *.05f;
//
//                 auto& shoulders = h.get_ref<glass::parts::shoulders>();
//                 // shoulders.right[1].angle.x += math::tau_8 * .61f;
//                 // shoulders.left[1].angle.x -= math::tau_8 * .61f;
//                 shoulders.right[0].angle.y += math::tau_8 * .01f;
//                 shoulders.right[2].angle.y += math::tau_8 * .4f;
//                 shoulders.right[2].angle.z -= math::tau_8 * .3f;
//                 shoulders.left[0].angle.y -= math::tau_8 * .01f;
//                 shoulders.left[2].angle.y += math::tau_8 * .4f;
//                 shoulders.left[2].angle.z += math::tau_8;
//
//                 auto& hips = h.get_ref<glass::parts::hips>();
//                 hips.right[0].angle.y += math::tau_8 *.1f;
//                 hips.right[2].angle.y += math::tau_8 *.05f;
//                 hips.left[0].angle.y -= math::tau_8 * .2f;
//                 hips.left[2].angle.y -= math::tau_8 * .1f;
//
//                 return h;
//             }
//         )
//     }.animate(hueman);
//
// constexpr auto floating = make_lines<0, 45, 90, 135, 180, 225, 270, 315>(floating_muscle_digest);

constexpr auto& drawn_model = walking_lines;

}  // namespace

template<typename Model, typename Paint>
void room::draw_model(const graphics::core& gl, const Model& model, const Paint& paint, const animation anim) const noexcept
{
    gl.prog.fill.use();
    constexpr color_t frames_display{ .3f, 1.f, .3f, .5f };
    gl.prog.fill.set_color(frames_display);

    const auto model_offset = gl.draw_size.x / (model.size() + 1);
    gl.prog.fill.set_view_transform(math::matrices::translate<float>({model_offset, gl.draw_size.y * .8f}));

    unsigned i = 0;
    for (const auto& it : model)
    {
        gl.prog.fill.set_transform(math::matrices::translate<float>({model_offset * i++, 0}));
        it.draw(gl);
    }

    constexpr auto scale_mat = math::matrices::uniform_scale(2.f);

    if (show_skin || show_blobs)
    {
        gl.prog.double_normal.use();
        gl::ActiveTexture(gl::TEXTURE0);

        gl.prog.double_normal.set_interpolation(anim.interpolation);
        gl.prog.double_normal.set_transform(scale_mat);
        gl.prog.double_normal.set_view_transform(math::matrices::translate(gl.draw_size / 2.f));
        gl.prog.double_normal.set_color({ 1, 1, 1 });
        const bool data = true;

        if (show_blobs)
        {
            gl::BindTexture(gl::TEXTURE_2D, debug_texture.id);
            gl.prog.double_normal.set_texture_mult(debug_texture.area);
            gl.prog.double_normal.set_texture_shift({0, 0});
            paint[anim.source % model.size()].draw(
                    gl.prog.double_normal,
                    paint[anim.dest % model.size()],
                    human_skin, data);
        }

        if (show_skin)
        {
            gl::BindTexture(gl::TEXTURE_2D, char_texture.id);
            gl.prog.double_normal.set_texture_mult(char_texture.area / 8.f);
            gl.prog.double_normal.set_texture_shift({facing / static_cast<float>(drawn_model.size()), 0});
            paint[anim.source % model.size()].draw(
                    gl.prog.double_normal,
                    paint[anim.dest % model.size()],
                    human_skin, data);
        }

        if (show_blobs)
        {
            gl::BindTexture(gl::TEXTURE_2D, debug_texture.id);
            gl.prog.double_normal.set_texture_mult(debug_texture.area);
            gl.prog.double_normal.set_texture_shift({0, 0});
            gl.prog.double_normal.set_color({ 1, 1, 1, .3f });
            paint[anim.source % model.size()].draw(
                    gl.prog.double_normal,
                    paint[anim.dest % model.size()],
                    human_skin, data);
        }
    }

    if (show_bones)
    {
        gl.prog.double_fill.use();
        gl.prog.double_fill.set_color({ 1, 1, 1, .7f });
        gl.prog.double_fill.set_interpolation(anim.interpolation);
        gl.prog.double_fill.set_transform(scale_mat);
        gl.prog.double_fill.set_view_transform(math::matrices::translate(gl.draw_size / 2.f));
        model[anim.source % model.size()].draw_interpolated(gl, model[anim.dest % model.size()]);
    }
}

void room::draw(const graphics::core& gl) const noexcept
{
    gl.prog.fill.use();
    gl.prog.fill.set_identity();
    gl.prog.fill.set_view_identity();
    constexpr color_t bg_color{ .3f, .5f, .4f };
    gl.prog.fill.set_color(bg_color);
    fill_screen(gl, gl.prog.fill);

    constexpr auto text_color = color_t::greyscale(.8f);

    gl.prog.text.use();
    gl.prog.text.set_color(text_color);
    draw_text<text_align::center, text_align::center>(*gl.fonts.title, gl.prog.text, "Model", gl.draw_size / 2.f, 32);

    gl::LineWidth(2.f);
    draw_model(gl,
        drawn_model[facing],
        walking_paint[facing],
        model_anim.load(std::memory_order_relaxed));

    gl.prog.fill.use();
    gl.prog.fill.set_identity();
    gl.prog.fill.set_view_identity();
    gl.prog.fill.set_color(graphics::black, .5f);

    gui.draw(gl);
}

std::optional<keyring::variant> room::step(const pointer_wrapper& pointer) noexcept
{
    auto work_copy = model_anim.load(std::memory_order_relaxed);
    timer += .12f / math::tau_2 * uni_time_factor;

    if (timer >= 1.f)
    {
        timer = 0.f;

        work_copy.source = work_copy.dest;

        if (++work_copy.dest >= drawn_model[0].size())
        {
            work_copy.dest = 0;
        }
    }

    work_copy.interpolation = timer;
    model_anim.store(work_copy, std::memory_order_relaxed);

    if (pointer.single_press)
    {
        if (const auto dest = gui.click<function>(pointer.cursor.pos, [](const auto&){}))
        {
            switch (*dest)
            {
                case function::exit_landing:
                    return { keyring::somewhere_else<hotel::landing::room>{} };

                case function::rotate_model:
                    if (static_cast<unsigned>(facing) + 1 >= drawn_model.size())
                    {
                        facing = 0;
                    }
                    else
                    {
                        ++facing;
                    }
                    break;

                case function::show_skin:
                    show_skin = !show_skin;
                    break;

                case function::show_blobs:
                    show_blobs = !show_blobs;
                    break;

                case function::show_bones:
                    show_bones = !show_bones;
                    break;

                case function::reload_images:
                    pictures.db.destroy_textures();
                    pictures.load_image("debug_tex.png", debug_texture);
                    pictures.load_image("octavia_tex.png", char_texture, gl::NEAREST);
                    break;

                default:
                    break;
            }
        }
    }

    return {};
}

void room::on_resize(const point_t screen_size) noexcept
{
    gui.resize(screen_size);
}

room::room() noexcept
{
    pictures.load_image("debug_tex.png", debug_texture);
    pictures.load_image("octavia_tex.png", char_texture, gl::NEAREST);
}

}  // namespace idle::hotel::model

#endif  // IDLE_COMPILE_GALLERY

