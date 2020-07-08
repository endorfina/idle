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

#include <cmath>
#include <random>
#include <idle/drawable.hpp>
#include <idle/hsv.hpp>

#include "room_landing.hpp"

namespace idle::hotel::landing
{
namespace
{

void draw_dim_noise(const graphics::noise_program_t& noise, const point_t size, const point_t& seed, const float alpha, const float fadeout)
{
    noise.use();
    noise.set_identity();
    noise.set_view_identity();

    const float vert[]
    {
        0, 0,
        size.x, 0,
        0, size.y,
        size.x, size.y
    };

    constexpr float faux_tex_coords[]
    {
        0, 0, 1, 0,
        0, 1, 1, 1
    };

    float white = 1, red = 1;

    if (fadeout < 1)
    {
        white = std::max<float>(0.f, (fadeout - .6f) / .4f);

        red = std::min<float>(1.f, fadeout / .6f);
    }

    const auto faster_alpha = std::min<float>(alpha * 3, 1);

    noise.set_color({white, white, white, std::min<float>(faster_alpha * 2, 1)});
    noise.set_secondary_color({
            math::sqr((alpha - .3f) / .7f) * red,
            .912f * red,
            .912f * red,
            faster_alpha});
    noise.set_tertiary_color({0, 0, 0, faster_alpha});
    noise.set_seed(seed);
    noise.position_vertex(vert);
    noise.texture_vertex(faux_tex_coords);
    gl::DrawArrays(gl::TRIANGLE_STRIP, 0, 4);
}

template<auto Size, class Rand>
void tickle_appendages(unsigned times, std::array<float, Size>& ray_array, Rand& rando)
{
    static_assert(Size > 0);
    if (times == 0) return;

    using random_int = std::uniform_int_distribution<unsigned int>;
    using random_float = std::uniform_real_distribution<float>;
    random_float amplitude{-.101f, .191f};
    random_int element_picker{0, Size - 1};

    do
    {
        const auto amp = amplitude(rando);
        const auto elem = element_picker(rando);
        ray_array[elem] += amp;
        ray_array[elem < 1 ? Size - 1 : elem - 1] += amp * .41f;
        ray_array[elem < 2 ? Size - 2 + elem : elem - 2] += amp * .13f;
        ray_array[elem >= Size - 1 ? 0 : elem + 1] += amp * .41f;
        ray_array[elem >= Size - 2 ? elem - Size + 2 : elem + 2] += amp * .13f;
    }
    while(!!--times);
}


template<auto Size, class Rand>
unsigned shift_appendages(std::array<float, Size>& ray_array, Rand& rando)
{
    static_assert(Size > 0);
    using random_int = std::uniform_int_distribution<unsigned int>;
    using random_float = std::uniform_real_distribution<float>;

    const float offset = random_float{-F_TAU_4, F_TAU_2}(rando);

    for (unsigned i = 0; i < Size; ++i)
    {
        const float val = std::sin(i * (F_TAU * 13.f / Size) + offset);
        const float amp = std::pow(val, 5);
        ray_array[i] = .061f * (val + amp);
    }

    tickle_appendages(13, ray_array, rando);

    return random_int{application_frames_per_second * 3, application_frames_per_second * 11}(rando);
}

}  // namespace

std::optional<keyring::variant> room::step(const pointer_wrapper& pointer)
{
    using random_float = std::uniform_real_distribution<float>;

    thing.rotation += .01f / application_frames_per_second;
    if (thing.rotation > F_TAU)
        thing.rotation -= F_TAU;

    if (!destination && !--thing.counter)
    {
        thing.counter = shift_appendages(thing.legs[0], fast_random_device);
    }

    for (unsigned i = 0; i < std::tuple_size<great_crimson_thing::arm_t>::value; ++i)
    {
        thing.legs[1][i] += (thing.legs[0][i] - thing.legs[1][i]) / application_frames_per_second * .3f;
    }

    random_float dist_float2{ 5000.f, 43758.5453f };
    std::generate(noise_seed.begin(), noise_seed.end(), [this, &dist_float2](){ return dist_float2(fast_random_device); });

    if (thing.alpha < 1.f)
    {
        thing.alpha = std::min<float>(thing.alpha + (clicked_during_intro ? .0091f : .00052f), 1.f);

        if (pointer.single_press)
        {
            clicked_during_intro = true;

            if (thing.alpha > .81f)
                destination = gui.click<keyring::variant>(pointer.cursor.pos);
        }
    }
    else if (destination)
    {
        thing.alpha = std::min<float>(thing.alpha + .002f, 2.f);

        if (thing.alpha >= 2.f)
        {
            return std::move(destination);
        }
    }
    else if (pointer.single_press)
    {
        destination = gui.click<keyring::variant>(pointer.cursor.pos);

        if (destination)
        {
            tickle_appendages(120, thing.legs[1], fast_random_device);
        }
    }

    return {};
}

void room::draw(const graphics::core& gl) const
{
    const auto alpha_sine = std::sin(std::min<float>(thing.alpha, 1.f) * F_TAU_4);
    gl.prog.fill.use();
    gl.prog.fill.set_identity();
    gl.prog.fill.set_view_identity();

    gl.prog.fill.set_color({
            .3f - .25f * alpha_sine,
            .3f - .27f * alpha_sine,
            .3f - .19f * alpha_sine,
            std::min<float>(alpha_sine * 1.41f, 1.f)});

    fill_screen(gl, gl.prog.fill);

    constexpr color_t dim_color{.021f, 0.f, .089f};
    gl.prog.gradient.use();
    gl.prog.gradient.set_color(dim_color, 0);
    gl.prog.gradient.set_secondary_color(dim_color, math::sqr(alpha_sine));
    gl.prog.gradient.set_view_transform(math::matrices::translate(point_t{gl.draw_size.x / 2.f, gl.draw_size.y / 2.f}));

    constexpr auto div = F_TAU / static_cast<float>(std::tuple_size<great_crimson_thing::arm_t>::value);
    constexpr auto array_len = std::tuple_size<great_crimson_thing::arm_t>::value + 2;

    constexpr std::array<float, array_len> black_interpolation_values = []()
    {
        std::array<float, array_len> out{};
        for (unsigned i = 1; i < array_len; ++i)
            out[i] = 1.f;
        return out;
    }();

    constexpr std::array<float, array_len> red_interpolation_values = []()
    {
        std::array<float, array_len> out{};
        out[0] = 1.1f;
        return out;
    }();

    const std::array<point_t, array_len> black_points = [this]()
    {
        std::array<point_t, array_len> out{};
        float fi = 0.f;

        std::transform(thing.legs[1].begin(), thing.legs[1].end(), out.begin() + 1,
                [&](float len) -> point_t
                {
                    len += 1.f;
                    fi += div;
                    return { std::cos(fi) * len, -std::sin(fi) * len };
                });

        out[array_len - 1] = out[1];
        return out;
    }();

    const auto lower_draw_size = std::min(gl.draw_size.x, gl.draw_size.y);
    const auto higher_draw_size = std::max(gl.draw_size.x, gl.draw_size.y);
    const float higher_ratio = std::max(4.f / 3.f, higher_draw_size / lower_draw_size);
    const float adaptive_mult = higher_ratio / (4.f / 3.f);

    gl.prog.gradient.set_transform(math::matrices::rotate(thing.rotation)
            * math::matrices::uniform_scale(lower_draw_size * (.8f - alpha_sine * .2f) * adaptive_mult));

    gl.prog.gradient.position_vertex(reinterpret_cast<const float *>(&black_points[0]));
    gl.prog.gradient.interpolation_vertex(reinterpret_cast<const float *>(black_interpolation_values.data()));
    gl::DrawArrays(gl::TRIANGLE_FAN, 0, array_len);

    constexpr auto ray_color = color_hsv(348, .91f, .86f * .51f);
    gl.prog.gradient.set_color(ray_color, 0);
    gl.prog.gradient.set_secondary_color(ray_color, .02f + .98f * alpha_sine);

    gl.prog.gradient.set_transform(math::matrices::rotate(thing.rotation) * math::matrices::uniform_scale(lower_draw_size * (.3f + .55f * alpha_sine) * adaptive_mult));
    gl.prog.gradient.interpolation_vertex(reinterpret_cast<const float *>(red_interpolation_values.data()));
    gl::DrawArrays(gl::TRIANGLE_FAN, 0, array_len);

    // TODO: Use a picture as the title
    // UPDATE: lmao, let's see about that!

    const auto fadeout_alpha_sine = std::sin(std::max<float>(thing.alpha + 1.f, 2.f) * F_TAU_4) + 1.f;

    if (alpha_sine > .95f && fadeout_alpha_sine > .8f)
    {
        const float gui_alpha = 20.f * (alpha_sine - .95f) * (fadeout_alpha_sine - .9f) / .1f;
        gl.prog.fill.use();
        gl.prog.fill.set_color({.71f, 0, 0, gui_alpha / 3});
        gl.prog.text.use();
        gl.prog.text.set_color({0, 0, 0, gui_alpha});
        gui.draw(gl);
    }

    gl.view_mask();

    draw_dim_noise(gl.prog.noise,
            gl.draw_size,
            *reinterpret_cast<const point_t*>(noise_seed.data()),
            alpha_sine,
            fadeout_alpha_sine);

    gl.view_normal();
}

void room::on_resize(point_t screen_size)
{
    gui.resize(screen_size);
}

}  // namespace idle::hotel::landing

