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
#include <idle/idle_guard.hpp>

namespace idle::hotel::landing
{
namespace
{

void draw_dim_noise(const graphics::noise_program_t& noise, const point_t size, const point_t seed, const float alpha, const float fadeout) noexcept
{
    noise.use();
    noise.set_identity();
    noise.set_view_identity();

    const float vert[] {
        0, 0,
        size.x, 0,
        0, size.y,
        size.x, size.y
    };

    constexpr float faux_tex_coords[] {
        0, 0, 1, 0,
        0, 1, 1, 1
    };

    float base_inner_color_val = 1,
          base_outer_color_val = 1,
          center_color_val = 1;

    const auto faster_alpha = std::min<float>(alpha * 3, 1);
    float base_color_alpha = std::min<float>(faster_alpha * 2.4f, 1);

    if (fadeout < 1)
    {
        base_outer_color_val = std::max<float>(0, fadeout * 2 - 1.f);
        base_inner_color_val = std::min<float>(1, fadeout * 2 + .2f);
        center_color_val = std::clamp<float>(fadeout * 1.8f - .6f, 0, 1);
    }

    noise.set_color(color_t::greyscale(base_inner_color_val, base_color_alpha));

    noise.set_secondary_color(color_t::greyscale(base_outer_color_val, base_color_alpha));

    noise.set_tertiary_color({ math::sqr((alpha - .3f) / .7f) * (center_color_val / .95f), center_color_val, center_color_val, faster_alpha });

    noise.set_quaternary_color(color_t::greyscale(0, faster_alpha));

    noise.set_seed(seed);
    noise.position_vertex(vert);
    noise.texture_vertex(faux_tex_coords);
    gl::DrawArrays(gl::TRIANGLE_STRIP, 0, 4);
}

template<auto Size, class Rand>
void tickle_appendages(unsigned times, std::array<float, Size>& ray_array, Rand& rando) noexcept
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
auto shift_appendages(std::array<float, Size>& ray_array, Rand& rando) noexcept -> unsigned
{
    static_assert(Size > 0);
    using random_int = std::uniform_int_distribution<unsigned int>;
    using random_float = std::uniform_real_distribution<float>;

    const float offset = random_float{-math::tau_4, math::tau_2}(rando);

    for (unsigned i = 0; i < Size; ++i)
    {
        const float val = std::sin(i * (math::tau * 13.f / Size) + offset);
        const float amp = std::pow(val, 5);
        ray_array[i] = .061f * (val + amp);
    }

    tickle_appendages(13, ray_array, rando);

    return random_int{application_frames_per_second * 3, application_frames_per_second * 11}(rando);
}

template<function Id, int X, int Y, unsigned width, unsigned height, typename Rando>
void cause_spark(luminous_cloud& cloud, const landing_button<Id, X, Y, width, height>& focus, Rando& rando) noexcept
{
    cloud.spark<width / 3, height / 4>(focus.pos, rando);
}


}  // namespace

template<unsigned rX, unsigned rY, typename Rando>
void luminous_cloud::spark(point_t position, Rando& rando) noexcept
{
    std::uniform_real_distribution<float> generator{ -1.f, 1.f };

    std::generate(table.begin(), table.end(), [&]()->flying_polyp
        {
            const auto direction = point_t{ generator(rando), generator(rando) };
            return
            {
                .fade_decr = ((generator(rando) - 1.f) * .00244f - .00398f) * uni_time_factor,
                .scale = (generator(rando) + 1.f) * 10.f + 60.f,
                .position = position + point_t{ generator(rando) * rX, generator(rando) * rY },
                .speed = point_t{ direction.x * .38f, direction.y * .23f } * uni_time_factor,
                .fade = 1.f,
                .distortion = color_t{ .5f + direction.x * .5f, .5f + direction.y * .5f, .5f, 0 }
            };
        });

    flag.store(true, std::memory_order_release);
}

void luminous_cloud::step() noexcept
{
    bool is_empty = true;

    for (auto& it : table)
    {
        if (it.fade > 0.f)
        {
            it.fade += it.fade_decr;
            it.position += it.speed;

            it.speed *= .99f;

            is_empty = false;
        }
    }

    if (is_empty)
        flag.store(false, std::memory_order_release);
}

auto room::step(const pointer_wrapper& pointer) noexcept -> std::optional<keyring::variant>
{
    using random_float = std::uniform_real_distribution<float>;

    thing.rotation += .01f / application_frames_per_second;
    if (thing.rotation > math::tau)
        thing.rotation -= math::tau;

    if (!destination && !--thing.counter)
    {
        thing.counter = shift_appendages(thing.legs[0], fast_random_device);
    }

    for (unsigned i = 0; i < std::tuple_size<great_crimson_thing::arm_t>::value; ++i)
    {
        thing.legs[1][i] += (thing.legs[0][i] - thing.legs[1][i]) * (.005f * uni_time_factor);
    }

    random_float dist_float2{ 5000.f, 43758.5453f };
    std::generate(noise_seed.begin(), noise_seed.end(),
            [this, &dist_float2](){ return dist_float2(fast_random_device); });
    random_float dist_float4{ -.75f, .75f };
    std::generate(menu_visual_noise.begin(), menu_visual_noise.end(),
            [this, &dist_float4](){ return dist_float4(fast_random_device); });

    if (polyps.flag.load(std::memory_order_relaxed))
    {
        polyps.step();
    }

    if (thing.alpha < 1.f)
    {
        thing.alpha = std::min<float>(thing.alpha + (impatient ? .0088f : .00059f) * uni_time_factor, 1.f);
    }

    if (destination)
    {
        constexpr float alpha_step = .0022f * uni_time_factor;
        thing.alpha = std::min<float>(thing.alpha + (impatient ? alpha_step * 4 : alpha_step /* * .2f */), 2.f);

        if (thing.alpha > 1.98f)
        {
            return std::move(destination);
        }
        else if (pointer.single_press)
        {
            impatient = true;
        }
    }
    else if (pointer.single_press)
    {
        const auto sparkler = [this](const auto& butt)
            {
                cause_spark(polyps, butt, fast_random_device);
            };

        if (thing.alpha < .821f)
        {
            impatient = true;
        }
        else if (const auto dest = gui.click<function>(pointer.cursor.pos, sparkler))
        {
            tickle_appendages(64, thing.legs[1], fast_random_device);

            impatient = false;
            focus = *dest;

            switch (focus)
            {
#ifdef IDLE_COMPILE_GALLERY
                case function::model:
                    destination.emplace(keyring::somewhere_else<hotel::model::room>{});
                    break;
#endif

                case function::start:
                    destination.emplace(keyring::somewhere_else<hotel::stage::room>{});
                    break;

                case function::cont:
                    destination.emplace(keyring::somewhere_else<hotel::landing::room>{});
                    break;

                default:
                    break;
            }
        }
    }

    return {};
}

void luminous_cloud::draw(const graphics::core& gl) const noexcept
{
    constexpr color_t not_white { 1, .91f, .91f, 0 };
    constexpr color_t not_red { 1.f, .15f, .31f, 0 };
    gl.prog.gradient.use();

    constexpr unsigned blob_array_len = 16;

    constexpr auto blob_colors = []() {
        std::array<float, blob_array_len> out{};
        out[0] = 1.f;
        return out;
    }();

    constexpr auto blob_points = []() {
        std::array<point_t, blob_array_len> out{};
        float angle = 0.f;
        static_assert(blob_array_len > 4);
        constexpr float step = math::tau / static_cast<float>(blob_array_len - 2);

        for (unsigned i = 1; i < blob_array_len; ++i)
        {
            out[i] = point_t{ math::const_math::cos(angle), math::const_math::sin(angle) };
            angle += step;
        }

        return out;
    }();

    gl.prog.gradient.position_vertex(reinterpret_cast<const float*>(blob_points.data()));
    gl.prog.gradient.interpolation_vertex(reinterpret_cast<const float*>(blob_colors.data()));

    for (const auto& it : table)
    {
        if (it.fade > 0.f)
        {
            const float alpha = (std::cos(math::tau_2 * (1.f + it.fade * 2)) + 1.f) / 6;
            const float scale = it.scale - it.fade * 45.f;

            gl.view_mask();
            gl.prog.gradient.set_color(not_white);
            gl.prog.gradient.set_secondary_color(not_white, alpha * .067f);
            gl.prog.gradient.set_view_transform(math::matrices::translate<float>(it.position));
            gl.prog.gradient.set_transform(math::matrices::uniform_scale<float>(scale));
            gl::DrawArrays(gl::TRIANGLE_FAN, 0, blob_array_len);

            gl.view_distortion();
            gl.prog.gradient.set_color(it.distortion);
            gl.prog.gradient.set_secondary_color(it.distortion, alpha * .5f);
            gl.prog.gradient.set_transform(math::matrices::uniform_scale<float>(scale / 7));
            gl::DrawArrays(gl::TRIANGLE_FAN, 0, blob_array_len);

            gl.view_normal();
            gl.prog.gradient.set_color(not_red);
            gl.prog.gradient.set_secondary_color(not_red * color_t::greyscale(it.fade), alpha);
            gl.prog.gradient.set_transform(math::matrices::uniform_scale<float>(scale / 4));
            gl::DrawArrays(gl::TRIANGLE_FAN, 0, blob_array_len);
        }
    }

    // for (const auto& it : small_table)
    // {
    //     if (it.fade > 0.f)
    //     {
    //         const float alpha = (std::cos(math::tau_2 * (1.f + it.fade * 2)) + 1.f) / 20;
    //         gl.view_distortion();
    //         gl.prog.gradient.set_color(it.distortion);
    //         gl.prog.gradient.set_secondary_color(it.distortion, alpha);
    //         gl.prog.gradient.set_view_transform(math::matrices::translate<float>(gl.draw_size / 2 +  it.position));
    //         gl.prog.gradient.set_transform(math::matrices::uniform_scale<float>(it.scale));
    //         gl::DrawArrays(gl::TRIANGLE_FAN, 0, blob_array_len);
    //     }
    // }
}

void room::draw(const graphics::core& gl) const noexcept
{
    const auto alpha_sine = std::sin(std::min<float>(thing.alpha, 1.f) * math::tau_4);
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

    constexpr auto div = math::tau / static_cast<float>(std::tuple_size<great_crimson_thing::arm_t>::value);
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

    const auto fadeout_alpha_sine = std::sin(std::max<float>(thing.alpha + 1.f, 2.f) * math::tau_4) + 1.f;

    gl.view_mask();

    draw_dim_noise(gl.prog.noise,
            gl.draw_size,
            *reinterpret_cast<const point_t*>(noise_seed.data()),
            alpha_sine,
            fadeout_alpha_sine);

    gl.view_normal();

    if (thing.alpha > .8f && fadeout_alpha_sine > .7f)
    {
        const button_state menu_state {
            .alpha = std::min<float>((thing.alpha - .8f) / .2f, 1.f) * ((fadeout_alpha_sine - .7f) / .3f),
            .focus = focus,
            .noise = menu_visual_noise.data()
        };
        gui.draw(gl, menu_state);
    }

    if (polyps.flag.load(std::memory_order_acquire))
    {
        polyps.draw(gl);
    }
}

void room::on_resize(const point_t screen_size) noexcept
{
    gui.resize(screen_size);
}

}  // namespace idle::hotel::landing

