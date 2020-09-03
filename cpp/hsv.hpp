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
#include <math.hpp>
#include "idle_defines.hpp"

namespace idle
{
namespace hsv
{

constexpr float compose(const float a, const float b, float c) noexcept
{
    if (c > 1.f)
        c -= 1.f;
    else if (c < 0.f)
        c += 1.f;
    if (6 * c < 1.f)
        return b + (a - b) * 6 * c;
    else if (2 * c < 1.f)
        return a;
    else if (3 * c < 2.f)
        return b + (a - b) * (.6666f - c) * 6;
    return b;
}

}  // namespace hsv

constexpr color_t color_hsv(float hue, float sat, float lum) noexcept
{
    hue /= 360;
    sat = lum < .5f ? lum * (1 + sat) : lum + sat - lum * sat;
    lum = 2 * lum - sat;

    return {
        hsv::compose(sat, lum, hue + .3333f),
        hsv::compose(sat, lum, hue),
        hsv::compose(sat, lum, hue - .3333f),
        1
    };
}

constexpr std::array<color_t, 6> rainbow_from_saturation(const float saturation) noexcept
{
    constexpr float hue_table [] = {
        0, 42, 60, 120, 255, 290
    };
    constexpr float lum_table [] = {
            .5f, .5f, .5f, .3f, .6f, .37f
    };

    std::array<color_t, 6> out{};

    for (unsigned i = 0; i < 6; ++i)
    {
        out[i] = color_hsv(hue_table[i], saturation, lum_table[i]);
    }
    return out;
}

}
