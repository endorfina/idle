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
#include <array>
#include <random>
#include <optional>
#include <idle/gl.hpp>
#include <idle/draw_text.hpp>
#include <idle/pointer_wrapper.hpp>
#include "gui.hpp"
#include "keys.hpp"

namespace idle::hotel::landing
{

struct great_crimson_thing
{
    using arm_t = std::array<float, 500>;

    float alpha = 0, rotation = 0;
    unsigned counter = 1;
    std::array<arm_t, 2> legs;
};

template<int X, int Y, unsigned width>
struct landing_button : gui::shapes::button<gui::positions::from_center<X, Y>, width, 28>
{
    void draw_foreground(const graphics::core& gl) const
    {
        gl.prog.text.use();
        draw_text<text_align::center, text_align::center>(gl, "???", this->pos, 20);
    }

#ifdef IDLE_COMPILE_GALLERY
    std::optional<keyring::variant> trigger() const
    {
        return { keyring::somewhere_else<hotel::model::room>{} };
    }
#endif
};


struct room
{
    using gui_t = gui::interface
        <
            landing_button<0, 20, 120>
        >;
    gui_t gui;

    std::minstd_rand fast_random_device;
    std::array<float, 2> noise_seed;
    bool clicked_during_intro = false;
    great_crimson_thing thing;

    void on_resize(point_t);

    std::optional<keyring::variant> step(const pointer_wrapper& cursor);

    void draw(const graphics::core&) const;
};

}  // namespace idle::hotel::landing
