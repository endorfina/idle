/*
    Copyright © 2020 endorfina <dev.endorfina@outlook.com>

    This file is part of Violet.

    Violet is free software: you can study it, redistribute it
    and/or modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation, either version 3 of
    the License, or (at your option) any later version.

    Violet is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Violet.  If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once
#include <mutex>
#include <atomic>
#include <vector>
#include <variant>
#include <unordered_map>
#include <math.hpp>
// #include <colony.hpp>
#include "gl.hpp"

class overlay;

namespace idle
{
enum Facedir : int8_t {
    Up = 0, Right_1_4 = 1, Right_side = 2, Right_3_4 = 3, Down = 4, Left_3_4 = 5, Left_side = 6, Left_1_4 = 7
};

class collection;

struct gui_elem
{
    enum class trigger_t {
        None,
#ifndef NDEBUG
        MakerRoom, AddBone, RemoveBone, SetTarget, Dragable, SelectBone, Save,
        SelectSkele, AddSkele
#endif
    } trigger = trigger_t::None;

    point_t bound;
    bool deactivated = false, hover = false;
    color_t bg, fg;

    using draw_proc_t = void (*)(const graphics::core&, point_t, const gui_elem&);
    draw_proc_t draw_proc = nullptr;

    static_assert(sizeof(int32_t) == sizeof(float));

    union {
        int32_t i[4];
        float f[4];
        const char* p[2];
    } data;

    std::string text;
};

class object
{
    friend class collection;
    object * next_obj_ptr = nullptr;
public:
    point_t pos;
    //float draw_scale = 1.f;
    std::atomic<float> depth = 0;

    // Boolean signifiers
    // bool is_solid = false, is_hidden = false, is_active = true, destroy_flag = false;
    bool is_hidden = false, destroy_flag = false, is_active = true;

    std::variant<gui_elem> data;

private:
    void step(collection&);

    void draw(const graphics::core&) const;

protected:
    void _apply_physics(void);
};

class collection
{
public:
    object * first_node = nullptr;
    // math::colony<object, 80, uint_fast8_t> data;
    std::unordered_map<object*, object*> replacements;
    std::mutex gen_access, node_access;

public:
    object& create_object(float x, float y);

    object& create_object(point_t pos);

    void destroy_all_objects();

    void sort_objects_by_depth();

    void iterate_object_step();

    void iterate_object_draw(const graphics::core&);

    void iterate_object_draw_special() const;

    // template<class T>
    // void for_each_object(T&& callable)
    // {
    //     for (auto& it : data)
    //         callable(it);
    // }

    // template<class T>
    // object* find_first_object(T&& predicate)
    // {
    //     for (auto& it : data)
    //         if (predicate(it))
    //             return &it;
    //     return nullptr;
    // }

};

struct light_source {
    double sx, sy;
    point_t pos;
    float radius;
    unsigned char power;

    light_source(const object * _parent, float x, float y, float r, unsigned char pow)
            : sx(_parent->pos.x), sy(_parent->pos.y), pos(x + _parent->pos.x, y + _parent->pos.y), radius(r), power(pow) {}
};

typedef std::vector<std::pair<unsigned, unsigned>> vn_choices;

class player_t {
    friend class red_room;
    object * possesed = nullptr;
    void step(::overlay & parent);

    enum class _action_enum_ { none, look, move, run }
    action = _action_enum_::none;
};
}
