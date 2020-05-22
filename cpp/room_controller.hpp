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
#include <variant>

#include "gl.hpp"
#include "room_variant.hpp"
#include "pointer.hpp"

namespace idle
{

template<typename Open>
struct door
{
    using opened_type = Open;

    template<typename Variant, typename...Args>
    void open(Variant& var, Args&&...args) const
    {
        var.template emplace<opened_type>(std::forward<Args>(args)...);
    }
};

template<typename SkipMonostate>
struct hotel
{
};

template<typename SkipMonostate, typename...Rooms>
struct hotel<std::variant<SkipMonostate, Rooms...>>
{
    std::optional<std::variant<door<Rooms>...>> rooms;
};

class controller
{
    hotel<room> next_variant;
    room current_variant;
    std::atomic_bool worker_active_flag{false};
    std::optional<std::thread> worker_thread;
    std::optional<std::string> crash_haiku;
    bool crashed{false};

public:
    pointer_keeper pointer;

    ~controller();

    void awake(bool);

    bool should_stay_awake() const;

    void join_worker();

    void resize(point_t size);

    void draw_frame(const graphics::core& gl);

    bool execute_pending_room_change(graphics::core&, const std::chrono::steady_clock::time_point& step_time);

    template<typename Room>
    void change_room()
    {
        awake(false);
        next_variant.rooms.emplace(door<Room>{});
    }

    void default_room_if_none_set();

    void signal_crash(std::string str);

    bool get_crashed() const;

    void do_step(graphics::core&);
};

}  // namespace idle
