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

#include <chrono>
#include <log.hpp>

#include "platform/display.hpp"
#include "room_variant.hpp"
#include "room_controller.hpp"
#include "draw_text.hpp"

namespace idle
{

namespace
{

void wait_one_frame(std::chrono::steady_clock::time_point& new_time)
{
    using namespace std::chrono_literals;
    constexpr auto minimum_elapsed_duration = std::chrono::duration_cast<
                        std::chrono::steady_clock::time_point::duration>(1.0s / APPLICATION_FPS);

    new_time += minimum_elapsed_duration;
    std::this_thread::sleep_until(new_time);
}


void dance(controller& ctrl, ::overlay& top, std::chrono::steady_clock::time_point step_time)
{
    if (ctrl.get_crashed())
        return;

    LOGD("Dancer open for business");

    while (ctrl.should_stay_awake())
    {
        wait_one_frame(step_time);
        ctrl.do_step(top);
    }
    LOGD("Dancer closed shop.");
}

TEMPLATE_CHECK_METHOD(on_resize);

TEMPLATE_CHECK_METHOD(step);

TEMPLATE_CHECK_METHOD(draw);

}  // namespace

controller::~controller()
{
    slumber();
    kill_during_sleep();
}

void controller::signal_crash(std::string str)
{
    crash_haiku.emplace(std::move(str));
    crashed = true;
}

bool controller::get_crashed() const
{
    return crashed;
}

void controller::wake(::overlay& top, const std::chrono::steady_clock::time_point& step_time)
{
    using namespace std::chrono_literals;
    constexpr auto skip_two_beats = std::chrono::duration_cast<
                        std::chrono::steady_clock::time_point::duration>(2.0s / APPLICATION_FPS);

    slumber();
    kill_during_sleep();
    worker_active_flag.store(true, std::memory_order_relaxed);
    worker_thread.emplace(dance, std::ref(*this), std::ref(top), step_time + skip_two_beats);
}

void controller::slumber()
{
    worker_active_flag.store(false, std::memory_order_relaxed);
}

void controller::kill_during_sleep()
{
    if (worker_thread)
    {
        worker_thread->join();
        worker_thread.reset();
    }
}

bool controller::should_stay_awake() const
{
    return worker_active_flag.load(std::memory_order_relaxed);
}

void controller::resize(point_t size)
{
    std::visit([size](auto& room){
        if constexpr (has_on_resize_method<TYPE_REMOVE_CVR(room)>::value)
            room.on_resize(size);
    }, current_variant);
}

void controller::do_step(::overlay& top)
{
    std::visit([&top](auto& room) {
        if constexpr (has_step_method<TYPE_REMOVE_CVR(room)>::value) {
            room.step(top);
        }
    }, current_variant);
}

void controller::draw_frame(const graphics::core& gl)
{
    if (crashed && crash_haiku)
    {
        gl.ptext.use();
        gl.ptext.set_color(1, 1, 1, .9f);

        draw_text<idle::TextAlign::Center, idle::TextAlign::Center>(gl, *crash_haiku,
                {gl.draw_size.x / 2.f, gl.draw_size.y / 2.f}, 32);
    }
    else
    {
        gl.pnormal.use();
        std::visit([&gl](auto& room){
            if constexpr (has_draw_method<TYPE_REMOVE_CVR(room)>::value)
            {
                room.draw(gl);
            }
        }, current_variant);
    }
}

bool controller::execute_pending_room_change(::overlay& top)
{
    if (room_current_id != room_next_id)
    {
        const char * const text = "Switching context to:";

        kill_during_sleep();
        room_current_id = room_next_id;

        switch (room_next_id)
        {
        case idle::room_id_enum::room_landing:
            LOGI("%s LANDING", text);
            current_variant.emplace<idle::landing_room>(top);
            break;

#ifdef COMPILE_M_ROOM
        case idle::room_id_enum::room_models:
            LOGI("%s MODEL", text);
            current_variant.emplace<idle::model_room>(top);
            break;
#endif

        // case idle::room_id_enum::room_red:
        //     LOGI("%s RED", text);
        //     current_variant.emplace<idle::red_room>(top);
        //     break;

        default:
            current_variant.emplace<std::monostate>();
            return false;
        }
    }
    return true;
}

void controller::change_room(room_id_enum id)
{
    slumber();
    room_next_id = id;
}

void controller::default_room_if_none_set()
{
    if (room_next_id == room_id_enum::room_uninitialized)
#ifdef COMPILE_M_ROOM
        room_next_id = room_id_enum::room_models;
#else
        room_next_id = room_id_enum::room_landing;
#endif
}

}  // namespace idle

