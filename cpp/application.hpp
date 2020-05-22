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
#include <chrono>
#include <optional>
#include <memory>
#include "gl.hpp"
#include "platform/context.hpp"

namespace isolation
{

struct pause_menu
{
    std::optional<graphics::render_buffer_t> buffers[2];
    float fadein_alpha = 0, shift = 0;

    void draw() const;
    void init();
};

struct application
{
private:
    bool update_display = false, blank_display = true;
    std::chrono::system_clock::time_point earliest_available_resize;
    std::chrono::steady_clock::time_point clock = std::chrono::steady_clock::now();

public:
    std::unique_ptr<pause_menu> pause;

    ::platform::context window;

private:
    bool execute_commands(bool nested);

public:
    static int real_main();

    bool load();

    void draw();
};

}  // namespace isolation
