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
#include <mutex>

#include "gl.hpp"
#include "hotel/variant.hpp"
#include <chry/hotel/service.hpp>
#include <chry/statistician.hpp>
#include "crash_handler.hpp"

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

template<typename T>
struct hotel_manager
{
};

template</* typename SkipMonostate,*/ typename...Rooms>
struct hotel_manager<std::variant</*SkipMonostate,*/ Rooms...>>
{
    std::optional<std::variant<door<Rooms>...>> rooms;
};

class controller
{
    hotel_manager<hotel::rooms> next_variant;
    hotel::rooms current_variant;
    hotel::room_service worker;
    point_t current_screen_size;
    std::mutex mutability;

public:
#ifdef IDLE_COMPILE_FPS_COUNTERS
    stats::statistician teller;
    stats::wall_clock tick_counter;
#endif
    std::atomic<platform::pointer> cached_cursor;
    crash_handler haiku;

    bool should_stay_awake() const;

    void sleep();

    void awaken(std::chrono::steady_clock::time_point step_time);

    void resize(point_t size);

    void draw_frame(const graphics::core& gl);

    std::optional<hotel::keyring::variant> do_step(const pointer_wrapper& cursor);
};

}  // namespace idle
