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
#include <string>
#include <optional>
#include <atomic>
#include <thread>

#include "gl.hpp"
#include "room_variant.hpp"

namespace idle
{

class controller
{
    idle::room_id_enum room_current_id = idle::room_id_enum::room_uninitialized, room_next_id = idle::room_id_enum::room_uninitialized;
    room current_variant;
    std::atomic_bool worker_active_flag{false};
    std::optional<std::thread> worker_thread;
    std::optional<std::string> crash_haiku;
    bool crashed{false};

public:
    ~controller();

    void wake(::overlay& top, const std::chrono::steady_clock::time_point& step_time);

    void slumber();

    bool should_stay_awake() const;

    void kill_during_sleep();

    void resize(point_t size);

    void draw_frame(const graphics::core& gl);

    bool execute_pending_room_change(::overlay& top);

    void change_room(room_id_enum id);

    void default_room_if_none_set();

    void signal_crash(std::string str);

    bool get_crashed() const;

    void do_step(::overlay& top);
};

}  // namespace idle
