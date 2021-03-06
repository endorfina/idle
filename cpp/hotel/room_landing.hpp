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
    void draw_foreground(const graphics::core& gl, const button_state& st) const noexcept
    {
        gl.prog.text.use();
        gl.prog.text.set_color({ .6f, 0, .35f, st.alpha });

        constexpr auto text = []() -> std::string_view
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

        const auto scale = static_cast<float>(height)
            * (Id == st.focus
                    ? 1.f + (1.f - st.alpha) * .15f
                    : .8f + st.alpha * .2f
                );

        draw_text<text_align::center, text_align::center>(*gl.fonts.title, gl.prog.text, text, this->pos, scale);

        const auto po = this->pos + point_t{ st.noise[0], st.noise[1] };

        gl.view_mask();
        gl.prog.text.set_color({ Id == st.focus ? 1.f - st.alpha : 0, 0, 0, st.alpha });
        draw_text<text_align::center, text_align::center>(*gl.fonts.title, gl.prog.text, text, po, scale + .85f + st.noise[3]);

        gl.view_normal();
    }

    auto trigger() const noexcept -> function
    {
        return Id;
    }
};

struct luminous_cloud
{
    struct flying_polyp
    {
        float fade_decr = 0.f, scale = 1.f;
        point_t position, speed;
        float fade = 0.f;
        color_t distortion;
    };

    std::array<flying_polyp, 80> table;
    std::atomic_bool flag = false;

    void step() noexcept;

    template<unsigned rX, unsigned rY, typename Rando>
    void spark(point_t position, Rando& rando) noexcept;

    void draw(const graphics::core& gl) const noexcept;
};

constexpr int menu_item_offset(const unsigned i)
{
#ifdef IDLE_COMPILE_GALLERY
    constexpr unsigned diff = 65;
#else
    constexpr unsigned diff = 85;
#endif
    return static_cast<int>(diff * i) - 40;
}

struct room
{
    using gui_t = gui::interface
        <
#ifdef IDLE_COMPILE_GALLERY
            landing_button<function::model, 0, menu_item_offset(2), 110, 38>,
#endif
            landing_button<function::start, 0, menu_item_offset(0), 90, 38>,
            landing_button<function::cont, 0, menu_item_offset(1), 130, 38>
        >;
    gui_t gui;

    std::minstd_rand fast_random_device{ std::random_device{}() };
    std::array<float, 2> noise_seed;
    std::array<float, 3> menu_visual_noise;
    function focus = function::none;
    bool impatient = false;
    std::optional<keyring::variant> destination;
    great_crimson_thing thing;
    luminous_cloud polyps;

    void on_resize(point_t) noexcept;

    auto step(const pointer_wrapper& cursor) noexcept -> std::optional<keyring::variant>;

    void draw(const graphics::core& gl) const noexcept;
};

}  // namespace idle::hotel::landing
