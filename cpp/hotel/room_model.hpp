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

#include <idle/gl.hpp>

#ifdef IDLE_COMPILE_GALLERY
#include <idle/pointer_wrapper.hpp>
#include <idle/draw_text.hpp>
#include "gui.hpp"
#include "keys.hpp"

namespace idle::hotel::model
{

struct animation
{
    unsigned source = 0, dest = 1;
    float interpolation = 0.f;
};

enum class function : unsigned char
{
    none, show_bones, show_skin, rotate_model, exit_landing
};

template<function Id, int X, int Y, unsigned width, unsigned height>
struct model_button : gui::shapes::button<gui::positions::edge_hugger<X, Y>, width, height>
{
    void draw_foreground(const graphics::core& gl) const noexcept
    {
        constexpr auto scheme = []() -> std::pair<std::string_view, color_t>
        {
            switch(Id)
            {
                case function::show_bones:
                    return { "bone", { 1.f, .5f, .5f } };

                case function::show_skin:
                    return { "skin", { .5f, 1.f, .5f } };

                case function::rotate_model:
                    return { "rot", { .5f, .5f, 1.f } };

                case function::exit_landing:
                    return { "land", { .5f, .5f, .5f } };

                default:
                    return { "???", { 1.f, 1.f, 1.f } };
            }
        }();

        gl.prog.text.use();
        gl.prog.text.set_color(scheme.second);

        draw_text<text_align::center, text_align::center>(*gl.fonts.regular, gl.prog.text, scheme.first, this->pos, height *.85f);
    }

    auto trigger() const noexcept -> function
    {
        return Id;
    }
};

struct room
{
    template<function Id, int X, int Y = -16>
    using control_button = model_button<Id, X, Y, 39, 22>;

    using gui_t = gui::interface
        <
            control_button<function::show_bones, -180>,
            control_button<function::show_skin, -140>,
            control_button<function::rotate_model, -100>,
            control_button<function::exit_landing, -25>
        >;
    gui_t gui;

    float timer = 0.f;
    unsigned char facing = 0;
    bool show_bones = false, show_skin = true;

    std::atomic<animation> model_anim;

    void on_resize(point_t) noexcept;

    std::optional<keyring::variant> step(const pointer_wrapper& cursor) noexcept;

    void draw(const graphics::core& gl) const noexcept;
};

}  // namespace idle::hotel::model

#endif  // IDLE_COMPILE_GALLERY

