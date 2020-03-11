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
#include <memory>
#include <shared_mutex>

#include "gl.hpp"
#include "objects.hpp"

#ifdef IDLE_COMPILE_GALLERY
#include "gui.hpp"
#include "draw_text.hpp"

namespace idle
{
struct model_room;

enum class Targeted : int32_t {
    GlobalRotation, GlobalScale, Length, Angle, LowerBound, UpperBound, Stiffness, Center
};

constexpr std::string_view get_enum_name(const Targeted t)
{
    switch (t) {
        case Targeted::Length:
            return "Length";
        case Targeted::Angle:
            return "Angle";
        case Targeted::LowerBound:
            return "Lower Bound";
        case Targeted::UpperBound:
            return "Upper Bound";
        case Targeted::Center:
            return "Center";
        case Targeted::Stiffness:
            return "Stiffness";
        default:
            return {};
    }
}

template<Targeted T, int X, int Y>
struct target_change : gui::shapes::rectangle<X, Y, 60, 14>
{
    void draw(const graphics::core& gl) const
    {
        gl.ptext.use();
        draw_text<TextAlign::Center, TextAlign::Center>(gl, get_enum_name(T),
                {this->pos.x + 30, this->pos.y + 7}, 14);
    }

    void trigger(model_room& r) const;
};

template<int P, int X, int Y>
struct target_dragable_position : gui::shapes::ellipse<X, Y, 24, 24>
{
    void draw(const graphics::core& gl) const
    {
        static_assert(P >= 0 && P <= 2);
        constexpr auto text = []()->std::string_view
        {
            if constexpr (P == 0)
                return "X";
            else if constexpr (P == 1)
                return "Y";
            else
                return "Z";
        }();

        gl.ptext.use();
        draw_text<TextAlign::Center, TextAlign::Center>(gl, text, this->pos, 14);
    }

    void trigger(model_room& r);
};

template<Targeted T, int X, int Y>
struct target_dragable_target : gui::shapes::ellipse<X, Y, 24, 24>
{
    void draw(const graphics::core& gl) const
    {
        constexpr auto text = []()->std::string_view
        {
            if constexpr (T == Targeted::GlobalRotation)
                return "Rot";
            else
                return "Sc";
        }();

        gl.ptext.use();
        draw_text<TextAlign::Center, TextAlign::Center>(gl, text, this->pos, 14);
    }

    void trigger(model_room& r);
};

template<int X, int Y>
struct button_add : gui::shapes::ellipse<X, Y, 24, 24>
{
    void draw(const graphics::core& gl) const
    {
        gl.ptext.use();
        draw_text<TextAlign::Center, TextAlign::Center>(gl, "+", this->pos, 20);
    }

    void trigger(model_room& r) const;
};

template<int X, int Y>
struct button_remove : gui::shapes::ellipse<X, Y, 24, 24>
{
    void draw(const graphics::core& gl) const
    {
        gl.ptext.use();
        draw_text<TextAlign::Center, TextAlign::Center>(gl, "-", this->pos, 20);
    }

    void trigger(model_room& r) const;
};

using model_edit_interface_t = gui::interface<
    target_change<
        Targeted::Center, 5, 60
    >,
    target_change<
        Targeted::Length, 5, 75
    >,
    target_change<
        Targeted::Angle, 5, 90
    >,
    target_change<
        Targeted::LowerBound, 5, 105
    >,
    target_change<
        Targeted::UpperBound, 5, 120
    >,
    target_dragable_position<
        0, 100, 200
    >,
    target_dragable_position<
        1, 125, 200
    >,
    target_dragable_position<
        2, 150, 200
    >,
    target_dragable_target<
        Targeted::GlobalRotation, 112, 220
    >,
    target_dragable_target<
        Targeted::GlobalScale, 138, 220
    >
>;

using model_interface_t = std::variant<
    model_edit_interface_t
>;


// struct working_model
// {
//     std::unordered_map<std::string, models::skeleton> skeleton_map;
//     models::skeleton* edited_skele = nullptr;
//     models::skeleton skele;
//     unsigned bid = 0;
//     float center_rotation = 0, z_rotation = 0.f, model_scale = 1.f;
//     std::unordered_map<unsigned, mat4x4_noopt_t> matrix_cache;
//     point_t center_translation;
//
//     models::bone* get_current_bone();
//     void refresh_matrix_cache();
//     void clear_from_matrix_cache(unsigned);
//     void recalibrate_center();
//     bool has_model() const;
//     void draw(const graphics::core& gl) const;
//     void add_bone();
//     void remove_bone();
// };

struct model_room
{
    model_room(graphics::core&);

    bool step(graphics::core&);

    void draw(const graphics::core& gl);

    void on_resize(point_t size);

    std::shared_mutex mutex;
    model_interface_t face;
    Targeted targeted;
    // working_model model_;

    struct _drag_struct_ {
        point_t* ptr = nullptr;
        float* ptr_val = nullptr;
        point_t start;
        float initial_value, animation;
    } drag;

    void start_drag(point_t& ptr, float point_3d_t::* const sel);
};

template<auto I>
constexpr float point_3d_t::* __choose_3d_point_value__()
{
    static_assert(I >= 0 && I < 3, "3D point index outside of 0 to 2 range.");
    if constexpr (I == 0)
        return &point_3d_t::x;
    else if constexpr (I == 1)
        return &point_3d_t::y;
    else
        return &point_3d_t::z;
}

template<int P, int X, int Y>
void target_dragable_position<P, X, Y>::trigger(model_room& r)
{
    // if (!r.model_.has_model()) return;
    r.start_drag(this->pos, __choose_3d_point_value__<P>());
}

template<Targeted T, int X, int Y>
void target_dragable_target<T, X, Y>::trigger(model_room& r)
{
    // if (!r.model_.has_model()) return;
    // start_drag
    r.drag.ptr = &this->pos;
    r.drag.start = this->pos;
    r.drag.ptr_val = nullptr;
    r.drag.animation = 0.f;

    // if constexpr (T == Targeted::GlobalRotation)
    //     r.drag.ptr_val = &r.model_.z_rotation;
    // else {
    //     static_assert(T == Targeted::GlobalScale);
    //     r.drag.ptr_val = &r.model_.model_scale;
    // }

    r.drag.initial_value = *r.drag.ptr_val;
}

template<Targeted T, int X, int Y>
void target_change<T, X, Y>::trigger(model_room& r) const
{
    // r.targeted = T;
}

template<int X, int Y>
void button_add<X, Y>::trigger(model_room& r) const
{
    // r.model_.add_bone();
}

template<int X, int Y>
void button_remove<X, Y>::trigger(model_room& r) const
{
    // r.model_.remove_bone();
}

}
#endif  // IDLE_COMPILE_GALLERY

