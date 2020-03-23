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
#include "gl.hpp"
#include "pointer.hpp"
#include "platform/display.hpp"
#include "pause.hpp"

namespace isolation
{
class application
{
    idle::pointer pointer;
    bool update_display = false;
    std::chrono::system_clock::time_point last_resize;

public:
    std::optional<idle::pause_menu> pause;

    ::platform::window window;

private:
    bool execute_commands(bool nested);

public:
    template<typename...Vars>
    application(Vars&&...vars)
        : window(std::forward<Vars>(vars)...)
    {}

    int real_main();

    bool load();

    void draw();
};

}  //namespace isolation
