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

#include "room_model.hpp"

#ifdef COMPILE_M_ROOM
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <memory>
#include "platform/asset_access.hpp"
#include "top.hpp"
#include "draw_text.hpp"
#include "these_bones.hpp"
#include "objects.hpp"
#include "hsv.hpp"
#include "io.hpp"
#include "blue/script.hpp"

namespace idle
{
namespace
{

constexpr bool is_3d_point(Targeted i)
{
    return i == Targeted::Angle || i == Targeted::LowerBound || i == Targeted::UpperBound || i == Targeted::Center;
}

template<class T>
T choose(T mn, T mid, T mx)
{
    if (mid > mx)
        return mx;
    else if (mid < mn)
        return mn;
    return mid;
}

constexpr int drag_grid_width = 15;

point_t floor_grid0(point_t p)
{
    const float x = static_cast<float>(choose<int>(0, int(p.x + drag_grid_width * 4.5f) / drag_grid_width, 8) - 4);
    const float y = static_cast<float>(choose<int>(0, int(p.y + drag_grid_width * 5.5f) / drag_grid_width, 10) - 5);
    return {x, y};
}

blue::dictionary skeleton_to_blue_dict(const models::skeleton& skele)
{
    blue::dictionary out;
    const auto& bag = skele.bag();

    if (skele.center.x != 0.f || skele.center.y != 0.f || skele.center.z != 0.f)
    {
        blue::list point;
        point.push_back(skele.center.x);
        point.push_back(skele.center.y);
        point.push_back(skele.center.z);
        out.push("center", std::move(point));
    }

    blue::list pile;
    for (const auto& bone : bag)
    {
        blue::dictionary dict;
        blue::list point;
        point.push_back(bone.angle.x);
        point.push_back(bone.angle.y);
        point.push_back(bone.angle.z);
        dict.push("len", bone.length);
        dict.push("angle", std::move(point));

        if (bone.parent_id < bag.size())
        {
            dict.push("parent", static_cast<long>(bone.parent_id));
        }
        pile.push_back(std::move(dict));
    }
    out.push("pile", std::move(pile));
    return out;
}

void _write_to_fid_(void * param, std::string_view view)
{
    ::write(*reinterpret_cast<int*>(param), view.data(), view.size());
}


} // namespace

void model_room::start_drag(point_t& ptr, float point_3d_t::* const sel)
{
    drag.start = ptr;
    drag.ptr = &ptr;
    drag.ptr_val = nullptr;
    drag.animation = 0.f;

    switch (targeted)
    {
        case Targeted::Angle:
            drag.ptr_val = &(model_.get_current_bone()->angle.*sel);
            break;

        case Targeted::LowerBound:
            drag.ptr_val = &(model_.get_current_bone()->lower_bound.*sel);
            break;

        case Targeted::UpperBound:
            drag.ptr_val = &(model_.get_current_bone()->upper_bound.*sel);
            break;

        case Targeted::Center:
            drag.ptr_val = &(model_.skele.center.*sel);
            break;

        case Targeted::Length:
            drag.ptr_val = &model_.get_current_bone()->length;
            break;

        case Targeted::Stiffness:
            drag.ptr_val = &model_.get_current_bone()->stiffness;
            break;

        default:
            LOGE("Unknown enum");
            std::abort();
    }

    drag.initial_value = *drag.ptr_val;
}

void option_save(model_room& r)
{

}

// void option_remove_bone(model_room& r)
// {
//     if (auto bn = r.model_.skele.bone_pile.extract(r.model_.bid))
//     {
//         for (auto& [uid, bon] : r.model_.skele.bone_pile)
//         {
//             if (bon.joint == &bn.mapped())
//             {
//                 bon.joint = bn.mapped().joint;
//             }
//         }
//         r.model_.clear_from_matrix_cache(r.model_.bid);
//         // r.remove_bone_label(parent, bid);
//         r.model_.refresh_matrix_cache();
//     }
// }

// void model_room::menu(collection& coll, math::point2<int> size)
// {
//     constexpr point_t standard_button {16, 16};
//     constexpr auto rainbow = rainbow_from_saturation(.5f);
//     unsigned i = 0;
//
//     switch (menu_id)
//     {
//     case menu_t::model_edit:
//         create_gui_elem(coll, {100, 5}, {40.f, standard_button.y}, gui_elem::trigger_t::Save, button_save, false, {.6f, .8f, .05f});
//         create_gui_elem(coll, {10, 30}, standard_button, gui_elem::trigger_t::AddBone, button_draw_plus, false, {.2f, .8f, .3f});
//         create_gui_elem(coll, {30, 30}, standard_button, gui_elem::trigger_t::RemoveBone, button_draw_plus, false, {.3f, .2f, .8f}).data.i[0] = 1;
//
//         // for (const auto& [id, bon] : skele.bone_pile)
//         //     create_gui_elem(coll, {size.x * .4f, -50.f}, {16, 16}, gui_elem::trigger_t::SelectBone, button_number, true, {.8f, .2f, .7f, .6f})
//         //         .data.i[0] = id;
//         fit_bone_labels(coll, size);
//         break;
//
//     case menu_t::model_pick:
//         for (const auto& [label, skeleton] : skeleton_map)
//         {
//             const float yy = std::min<float>(40.f, static_cast<float>((size.y - 20) / skeleton_map.size()));
//             auto& elem = create_gui_elem(coll, {5, 10.f + i * yy}, {200, yy * .9f},
//                 gui_elem::trigger_t::SelectSkele, button_label, false, rainbow[i % rainbow.size()]);
//             elem.text = label;
//             ++i;
//         }
//         create_gui_elem(coll, {300, 50}, {60, 60}, gui_elem::trigger_t::AddSkele, button_draw_plus, false, {.2f, .8f, .3f});
//         break;
//     }
// }


models::bone* working_model::get_current_bone()
{
    if (bid < skele.size())
        return &skele[bid];
    return nullptr;
}

void working_model::add_bone()
{
    auto& bon = skele.add_bone_to_the_bag();
    bon.length = 20.f;

    if (bid < skele.size())
    {
        bon.parent_id = bid;
    }

    bid = skele.size() - 1;
    refresh_matrix_cache();
    // r.recalibrate_center();
    // TODO: Add bone selector
    // fit_bone_labels(parent.collection, parent.gl.draw_size);
}

void working_model::remove_bone()
{
    skele.remove_bone(bid);
    bid = skele.size();
}

// void model_room::recalibrate_center()
// {
//     point_t mi{}, ma{};
//     mat4x4_noopt_t view = mat4x4_noopt_t::rotate_x(F_TAU_4) * mat4x4_noopt_t::rotate_y(z_rotation) * mat4x4_noopt_t::scale(model_scale, model_scale, 0);
//
//     for (const auto& [id, mat] : matrix_cache)
//     {
//         const point_t k = (mat * view) * point_t{1, 0};
//         if (k.x > ma.x)
//             ma.x = k.x;
//         if (k.x < mi.x)
//             mi.x = k.x;
//         if (k.y > ma.y)
//             ma.y = k.y;
//         if (k.y < mi.y)
//             mi.y = k.y;
//     }
//     center_translation = (mi + ma) / -2;
// }

bool working_model::has_model() const
{
    return !!skele.size();
}

void working_model::refresh_matrix_cache()
{
    for (unsigned i = 0; i < skele.size(); ++i)
        if (matrix_cache.find(i) == matrix_cache.end())
        {
            const auto& bon = skele[i];
            mat4x4_noopt_t mat = mat4x4_noopt_t::scale(bon.length) * mat4x4_noopt_t::rotate_x(bon.angle.x) * mat4x4_noopt_t::rotate_y(bon.angle.y) * mat4x4_noopt_t::rotate(bon.angle.z);

            for (const models::bone* b = &skele[bon.parent_id]; b; b = &skele[b->parent_id])
            {
                mat *= mat4x4_noopt_t::translate(b->length, 0)
                    * mat4x4_noopt_t::rotate_x(b->angle.x) * mat4x4_noopt_t::rotate_y(b->angle.y) * mat4x4_noopt_t::rotate(b->angle.z);
            }
            matrix_cache.try_emplace(i, mat);
        }
}

void working_model::clear_from_matrix_cache(const unsigned key)
{
    if (const auto bn = matrix_cache.extract(key); bn && key < skele.size())
    {
        for (unsigned i = 0; i < skele.size(); ++i)
        {
            if (key == skele[i].parent_id)
            {
                clear_from_matrix_cache(i);
            }
        }
    }
}

void working_model::draw(const graphics::core& gl) const
{
    constexpr point_t crosshair[4]{{-50, 0}, {50, 0}, {0, -50}, {0, 50}};
    gl.pfill.use();
    gl.pfill.set_view_transform(mat4x4_t::translate(gl.draw_size.x / 2.f, gl.draw_size.y / 2.f));
    gl.pfill.set_transform(mat4x4_t::rotate(center_rotation) * mat4x4_t::scale(100));
    gl.pfill.set_color(1, .1f, .2f, .5f);
    gl::LineWidth(3.f);
    gl.pfill.position_vertex(reinterpret_cast<const GLfloat*>(crosshair));
    gl::DrawArrays(gl::LINES, 0, 4);

    constexpr point_t bone_line[2]{{0, 0}, {1, 0}};
    const mat4x4_noopt_t view_mat = mat4x4_noopt_t::rotate_x(F_TAU_4) * mat4x4_noopt_t::rotate_y(z_rotation)
            * mat4x4_noopt_t::scale(model_scale, model_scale, 0) * mat4x4_noopt_t::translate(center_translation + point_t{gl.draw_size.x / 2.f, gl.draw_size.y / 2.f});
    gl.pfill.set_view_transform(view_mat);
    gl.pfill.position_vertex(reinterpret_cast<const GLfloat*>(bone_line));

    gl::LineWidth(2.f);
    gl.pfill.set_color(1, .2f, 1, .6f);
    gl.pfill.set_transform(mat4x4_noopt_t::scale(10, 1, 1) * mat4x4_noopt_t::translate(skele.center));
    gl::DrawArrays(gl::LINES, 0, 2);

    gl.pfill.set_color(1, 1, 1, .8f);

    for (unsigned i = 0; i < skele.size(); ++i)
    {
        gl.pfill.set_transform(matrix_cache.at(i));
        gl::DrawArrays(gl::LINES, 0, 2);
    }

    if (const auto f = matrix_cache.find(bid); f != matrix_cache.end())
    {
        gl.pfill.set_transform(mat4x4_t::rotate(F_TAU_8));
        const auto mat = f->second * view_mat;
        const std::pair<point_t, color_t> array[2] {
            {mat * bone_line[0], {.6f, 1, .1f, .6f}},
            {mat * bone_line[1], {1, .1f, .9f, .6f}}
        };
        for (const auto& [cs, col] : array)
        {
            gl.pfill.set_view_transform(mat4x4_t::translate(cs));
            gl.pfill.set_color(col);
            fill_rectangle(gl.pfill, {-3, -3, 3, 3});
        }
    }

    gl::LineWidth(2.f);

    if (bid < skele.size())
    {
        const models::bone& bone = skele[bid];
        char str[350];
        gl.ptext.use();
        gl.ptext.set_color(1,1,1,1);
        draw_text<idle::TextAlign::Near, idle::TextAlign::Far>(gl, {str, static_cast<size_t>(snprintf(str, sizeof(str),
                "Rot: %.2f pi, Sc: %.2f\ncenter [%.2f, %.2f, %.2f]\nBID[%u] {\nLength: %.2f\nAngle [%.2f, %.2f, %.2f]\nLower [%.2f, %.2f, %.2f]\nUpper [%.2f, %.2f, %.2f]\nStiffness: %.2f\n}",
                z_rotation / F_PI, model_scale,
                skele.center.x, skele.center.y, skele.center.z,
                bid, bone.length,
                bone.angle.x, bone.angle.y, bone.angle.z,
                bone.lower_bound.x, bone.lower_bound.y, bone.lower_bound.z,
                bone.upper_bound.x, bone.upper_bound.y, bone.upper_bound.z,
                bone.stiffness))},
            {5, static_cast<float>(gl.draw_size.y)}, 12);
        gl.pfill.use();
    }
}



void model_room::on_resize(point_t size)
{
    std::unique_lock lock(mutex);
    std::visit([size](auto& f) { f.resize(size); }, face);
}

model_room::model_room(::overlay& parent)
{
    // if (const auto modelfile = platform::asset::hold(parent.db.file_models.c_str()))
    // {
    //     model_.skeleton_map = models::load(modelfile.view());
    //     // if (!!skeleton_map.size())
    //     // {
    //     //     edited_skele = &skeleton_map.begin()->second;
    //     //     skele = *edited_skele;
    //     //     refresh_matrix_cache();
    //     // }
    // }
    std::visit([&parent](auto& f) { f.resize({float(parent.gl.draw_size.x), float(parent.gl.draw_size.y)}); }, face);
}

bool model_room::step(::overlay& parent)
{
    const auto pointer = parent.get_pointer();
    std::unique_lock lock(mutex);
    if (drag.ptr_val)
    {
        if (pointer.pressed)
        {
            const auto [factor, add] = floor_grid0(*drag.ptr = pointer.pos - drag.start);
            switch (targeted)
            {
                case Targeted::GlobalRotation:
                case Targeted::Angle:
                case Targeted::LowerBound:
                case Targeted::UpperBound:
                    {
                        const auto dest = choose<float>(-F_PI, drag.initial_value - add * powf(2, factor) / F_PI, F_PI);
                        if (abs(dest - *drag.ptr_val) < .033f)
                            *drag.ptr_val = dest;
                        else
                            *drag.ptr_val += (dest - *drag.ptr_val) / 4;
                    }
                    break;

                case Targeted::GlobalScale:
                    *drag.ptr_val = choose<float>(.1f, drag.initial_value - add * powf(2, factor), 10);
                    break;

                default:
                    *drag.ptr_val = choose<float>(-200, drag.initial_value - add * powf(2, factor), 200);
            }
        }
        else {
            *drag.ptr = drag.start;
            drag.ptr = nullptr;
            drag.ptr_val = nullptr;
        }
        drag.animation += (.6f - drag.animation) / 12;
        model_.clear_from_matrix_cache(model_.bid);
        model_.refresh_matrix_cache();
        //recalibrate_center();
    }
    else if (pointer.single_press)
    {
        std::visit([this, &pointer](auto& f) { f.click(*this, pointer.pos); }, face);
    }

    //         case gui_elem::trigger_t::SelectBone:
    //             bid = gui.data.i[0];
    //             break;
    //         case gui_elem::trigger_t::SelectSkele:
    //             edited_skele = &skeleton_map.at(gui.text);
    //             skele = *edited_skele;
    //             menu_id = menu_t::model_edit;
    //             menu(parent.collection, parent.gl.draw_size);
    //             refresh_matrix_cache();
    //             break;
    //         case gui_elem::trigger_t::AddSkele:
    //             {
    //                 std::string str("new");
    //                 str.resize(16);
    //                 math::generate_random_string(str.data() + 3, str.size() - 3);
    //                 edited_skele = &skeleton_map.try_emplace(std::move(str)).first->second;
    //             }
    //             skele = *edited_skele;
    //             menu_id = menu_t::model_edit;
    //             menu(parent.collection, parent.gl.draw_size);
    //             refresh_matrix_cache();
    //             break;
    //         case gui_elem::trigger_t::Save:
    //             save_all(parent.db);
    //             break;
    //         default:
    //             break;
    //     }
    // }
    model_.center_rotation += .0003f;
    if (model_.center_rotation > F_TAU)
        model_.center_rotation -= F_TAU;

    return true;
}

void model_room::draw(const graphics::core& gl)
{
    std::shared_lock lock(mutex);
    model_.draw(gl);

    if (drag.ptr)
    {
        constexpr point_t dgrid[]{
            {-5, 5}, {6, 5},
            {-5, 4}, {6, 4},
            {-5, 3}, {6, 3},
            {-5, 2}, {6, 2},
            {-5, 1}, {6, 1},
            {-5, 0}, {6, 0},
            {-5, -1}, {6, -1},
            {-5, -2}, {6, -2},
            {-5, -3}, {6, -3},
            {-5, -4}, {6, -4},
            {5, -5}, {5, 6},
            {4, -5}, {4, 6},
            {3, -5}, {3, 6},
            {2, -5}, {2, 6},
            {1, -5}, {1, 6},
            {0, -5}, {0, 6},
            {-1, -5}, {-1, 6},
            {-2, -5}, {-2, 6},
            {-3, -5}, {-3, 6},
            {-4, -5}, {-4, 6}
        };
        gl.pfill.set_view_transform(mat4x4_t::scale(drag_grid_width * (.4f + drag.animation)) * mat4x4_t::translate(drag.start - point_t{drag_grid_width / 2, drag_grid_width / 2}));
        gl.pfill.set_identity();
        gl.pfill.set_color(.9f, .9f, .2f, drag.animation);
        gl.pfill.position_vertex(reinterpret_cast<const GLfloat*>(dgrid));
        gl::DrawArrays(gl::LINES, 0, sizeof(dgrid) / sizeof(point_t));

        const auto diff = floor_grid0(*drag.ptr);
        gl.pfill.position_vertex(square_coordinates);
        gl.pfill.set_transform(mat4x4_t::translate(diff));
        gl::DrawArrays(gl::TRIANGLE_STRIP, 0, 4);
    }

    gl.pfill.set_view_identity();
    gl::LineWidth(1.f);

    gl.pfill.set_color(color_t::greyscale(.15f, .75f));
    gl.ptext.use();
    gl.ptext.set_color(1, 1, 1);

    std::visit([&gl](const auto& f) { f.draw(gl); }, face);
}

void model_room::save_all(const Database& db)
{
    blue::dictionary dict;
    *model_.edited_skele = model_.skele;

    for (const auto& [name, sk] : model_.skeleton_map)
        dict.push(name, skeleton_to_blue_dict(sk));

    auto fn = "assets/" + db.file_models;
    io::file wf{ ::open(fn.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) };
    constexpr std::string_view prefix = u8"🍦💀";
    ::write(wf, prefix.data(), prefix.size());
    blue::print(std::move(dict), _write_to_fid_, &wf.fd);
}


}  // namespace idle

#endif  // COMPILE_M_ROOM

