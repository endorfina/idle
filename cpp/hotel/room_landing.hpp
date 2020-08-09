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
#include <atomic>
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

enum class function : unsigned char
{
    none, model, start, cont
};

struct button_state
{
    float alpha;
    function focus;
    const float* noise;
};

template<function Id, int X, int Y, unsigned width, unsigned height>
struct landing_button : gui::shapes::buttonless<gui::positions::from_center<X, Y>, width, height>
{
    void draw_foreground(const graphics::core& gl, const button_state& st) const
    {
        gl.prog.text.use();
        gl.prog.text.set_color({ .61f, 0, 0, st.alpha });

        constexpr auto text = []()->std::string_view
        {
            switch(Id)
            {
                case function::start:
                    return "Start";

                case function::cont:
                    return "Continue";

                case function::model:
                    return "Gallery";

                default:
                    return "???";
            }
        }();

        const float scale = Id == st.focus
                ? static_cast<float>(height) + (1.f - st.alpha) * 4
                : static_cast<float>(height - 1) + st.alpha;

        draw_text<text_align::center, text_align::center>(*gl.fonts.title, gl.prog.text, text, this->pos, scale);

        const auto po = this->pos + point_t{ st.noise[0], st.noise[1] };

        gl.view_mask();
        gl.prog.text.set_color({ Id == st.focus ? 1.f - st.alpha : 0, 0, 0, st.alpha });
        draw_text<text_align::center, text_align::center>(*gl.fonts.title, gl.prog.text, text, po, scale + .85f + st.noise[3]);

        gl.view_normal();
    }

    function trigger() const
    {
        return Id;
    }
};

struct luminous_cloud
{
    struct flying_polyp
    {
        float fade = 0.f, fade_decr = 0.f, scale = 1.f;
        point_t position, speed;
    };

    std::array<flying_polyp, 40> table;
    std::atomic_bool flag = false;

    void step();

    void draw(const graphics::core& gl) const;
};

struct room
{
    using gui_t = gui::interface
        <
#ifdef IDLE_COMPILE_GALLERY
            landing_button<function::model, 0, 120, 120, 38>,
#endif
            landing_button<function::start, 0, -20, 100, 38>,
            landing_button<function::cont, 0, 50, 100, 38>
        >;
    gui_t gui;

    std::minstd_rand fast_random_device{ std::random_device{}() };
    std::array<float, 2> noise_seed;
    std::array<float, 3> menu_visual_noise;
    function focus = function::none;
    bool clicked_during_intro = false;
    std::optional<keyring::variant> destination;
    great_crimson_thing thing;
    luminous_cloud polyps;

    void on_resize(point_t);

    std::optional<keyring::variant> step(const pointer_wrapper& cursor);

    void draw(const graphics::core&) const;
};

}  // namespace idle::hotel::landing
