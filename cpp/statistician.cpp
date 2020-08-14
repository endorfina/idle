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

#include <cstdio>
#include "statistician.hpp"
#include "draw_text.hpp"

namespace idle::stats
{

void statistician::count_fps(const std::chrono::high_resolution_clock::time_point& start_point)
{
    if (iter >= frame_count.size())
        iter = 0;

    auto& it = frame_count[iter++];
    it.x = static_cast<float>(iter) / static_cast<float>(frame_count.size() - 1);
    const auto now = std::chrono::high_resolution_clock::now();
    it.y = static_cast<float>(std::chrono::duration_cast<std::chrono::duration<double, std::micro>>(now - start_point) / time_minimum_elapsed) * .999f;
}

void statistician::draw_fps(const graphics::core& gl) const
{
    gl::LineWidth(1.f);

    gl.prog.fill.use();
    gl.prog.fill.set_color({1, 1, 1, 0.5f});
    gl.prog.fill.position_vertex(&frame_count.front().x);
    gl.prog.fill.set_transform(math::matrices::scale<float>(gl.draw_size));
    gl.prog.fill.set_view_identity();

    gl.view_mask();
    gl::DrawArrays(gl::LINE_STRIP, 0, frame_count.size());
    gl.view_normal();
    gl::DrawArrays(gl::LINE_STRIP, 0, frame_count.size());
}

void wall_clock::tick()
{
    if (iter++ >= application_frames_per_second)
    {
        const auto now = std::chrono::time_point_cast<duration_type>(std::chrono::steady_clock::now());
        const std::chrono::duration<double> diff = now - last_fps_measurement;
        const auto val = static_cast<double>(application_frames_per_second) / diff.count();

        const auto amt = std::snprintf(out_str.data(), out_str.size(), "%.3lf", val);
        if (amt < 0)
        {
            view.remove_suffix(view.size());
        }
        else
        {
            view = { out_str.data(), static_cast<size_t>(amt) };
        }

        last_fps_measurement = now;
        iter = 0;
    }
}

void wall_clock::draw_fps(const graphics::core& gl) const
{
    constexpr point_t fps_draw_point{10.f, 10.f};
    gl.prog.text.use();
    gl.prog.text.set_color({1, .733f, .496f, .91f});
    gl.view_mask();
    draw_text<text_align::near, text_align::near>(
            *gl.fonts.title, gl.prog.text, view, fps_draw_point, 10);
    gl.view_normal();
    draw_text<text_align::near, text_align::near>(
            *gl.fonts.title, gl.prog.text, view, fps_draw_point, 10);
}

}  // namespace idle::stats
