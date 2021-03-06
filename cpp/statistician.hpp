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
#include <thread>
#include <chrono>
#include <string_view>
#include "gl.hpp"


namespace idle::stats
{

constexpr auto time_one_second = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::seconds(1));
constexpr auto time_minimum_elapsed = time_one_second / application_frames_per_second;

class statistician
{
    using array_type = std::array<point_t, idle::application_frames_per_second * 12>;

    array_type frame_count;
    unsigned iter = 0;

public:
    void count_fps(const std::chrono::high_resolution_clock::time_point start_point) noexcept;

    void draw_fps(const graphics::core& gl) const noexcept;

    constexpr statistician() noexcept
        : frame_count([]{
                    statistician::array_type out;
                    // TODO: change back to fill()
                    for (auto& it : out)
                        it = {.5f, 2.f};
                    return out;
                }())
    {
    }
};

class wall_clock
{
    using duration_type = std::chrono::duration<double>;
    using time_point = std::chrono::time_point<std::chrono::steady_clock, duration_type>;

    time_point last_fps_measurement = std::chrono::time_point_cast<duration_type>(std::chrono::steady_clock::now());
    unsigned iter = 0;
    std::array<char, 24> out_str;
    std::string_view view;

public:
    void tick() noexcept;

    void draw_fps(const graphics::core& gl) const noexcept;
};

}  // namespace idle::stats
