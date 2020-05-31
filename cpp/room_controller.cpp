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

#include "room_variant.hpp"
#include "room_controller.hpp"
#include "draw_text.hpp"

namespace idle
{

namespace
{

auto wait_one_frame(std::chrono::steady_clock::time_point new_time)
{
    using namespace std::chrono_literals;
    constexpr auto minimum_elapsed_duration = std::chrono::duration_cast<
                        std::chrono::steady_clock::time_point::duration>(1.0s / application_frames_per_second);

    new_time += minimum_elapsed_duration;
    std::this_thread::sleep_until(new_time);
    return new_time;
}


void dance(controller& ctrl, std::chrono::steady_clock::time_point step_time)
{
    if (ctrl.haiku.has_crashed())
        return;

    LOGD("Dancer open for business");

    while (ctrl.should_stay_awake())
    {
        step_time = wait_one_frame(step_time);
        ctrl.do_step();
    }
    LOGD("Dancer closed shop.");
}

TEMPLATE_CHECK_METHOD(on_resize);

TEMPLATE_CHECK_METHOD(step);

TEMPLATE_CHECK_METHOD(draw);

}  // namespace

controller::~controller()
{
    LOGD("controller::~controller");
    join_worker();
}

void crash_haiku::crash(std::string str)
{
    error_string = std::move(str);
    crashed.store(true, std::memory_order_release);
}

bool crash_haiku::has_crashed() const
{
    return crashed.load(std::memory_order_acquire);
}

const std::string& crash_haiku::get_string() const
{
    return error_string;
}

void controller::sleep()
{
    worker_active_flag.store(false, std::memory_order_relaxed);
}

void controller::join_worker()
{
    sleep();

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
    const std::lock_guard block_room_changes{mutability};
    current_screen_size = size;

    std::visit([size](auto& room)
    {
        if constexpr (has_on_resize_method<TYPE_REMOVE_CVR(room)>::value)
        {
            room.on_resize(size);
        }
    }, current_variant);
}

void controller::draw_frame(const graphics::core& gl)
{
    if (haiku.has_crashed())
    {
        gl.prog.text.use();
        gl.prog.text.set_color({1, 1, 1, .9f});

        draw_text<text_align::center, text_align::center>(gl, haiku.get_string(),
                math::point_cast<GLfloat>(gl.draw_size) / 2.f, 32);
    }
    else
    {
        gl.prog.normal.use();

        const std::lock_guard block_room_changes{mutability};

        std::visit([&gl](auto& room)
        {
            if constexpr (has_draw_method<TYPE_REMOVE_CVR(room)>::value)
            {
                room.draw(gl);
            }
        }, current_variant);
    }
}

void controller::awaken(const std::chrono::steady_clock::time_point clock)
{
    using namespace std::chrono_literals;
    constexpr auto skip_a_beat = std::chrono::duration_cast<
                        std::chrono::steady_clock::time_point::duration>(1.5s / application_frames_per_second);

    if (worker_thread)
    {
        sleep();
        worker_thread->join();
        worker_thread.reset();
    }

    worker_active_flag.store(true, std::memory_order_relaxed);
    worker_thread.emplace(dance, std::ref(*this), clock + skip_a_beat);
}

void controller::default_room_if_none_set()
{
    if (const auto ptr = std::get_if<std::monostate>(&current_variant); ptr && !next_variant.rooms)
    {
        next_variant.rooms.emplace(door<landing_room>{});
    }
}

namespace
{
template<typename T>
constexpr char room_label[] = "UNNAMED";

template<>
constexpr char room_label<landing_room>[] = "LANDING";

#ifdef COMPILE_GALLERY
template<>
constexpr char room_label<model_room>[] = "MODEL";
#endif

}  // namespace

void controller::do_step()
{
    std::visit([](auto& room) {
        if constexpr (has_step_method<TYPE_REMOVE_CVR(room)>::value) {
            room.step();
        }
    }, current_variant);

    if (next_variant.rooms)
    {
        const std::lock_guard block_drawing_and_resizing{mutability};

        std::visit(
            [this] (const auto& gate)
            {
                using T = typename TYPE_REMOVE_CVR(gate)::opened_type;
                LOGI("Switching context to: %s", room_label<T>);

                static_assert(std::is_constructible_v<T>);
                gate.open(current_variant);

                if constexpr (has_on_resize_method<T>::value)
                {
                    std::get<T>(current_variant).on_resize(current_screen_size);
                }
            },
            *next_variant.rooms);

        next_variant.rooms.reset();
    }
}

}  // namespace idle

