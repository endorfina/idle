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

#include "room_controller.hpp"
#include "draw_text.hpp"
#include "idle_guard.hpp"

namespace idle
{

namespace
{

auto wait_one_frame(std::chrono::steady_clock::time_point new_time) noexcept
{
    new_time += stats::time_minimum_elapsed;
    std::this_thread::sleep_until(new_time);
    return new_time;
}

template<typename T>
struct is_hotel_room : std::false_type {};

template<typename T>
struct is_hotel_room<hotel::keyring::somewhere_else<T>> : std::true_type {};

idle_check_method_boilerplate(draw);
idle_check_method_boilerplate(step);
idle_check_method_boilerplate(on_resize);

}  // namespace


void controller::sleep() noexcept
{
    worker.set_active(false);
}

void controller::resize(point_t size) noexcept
{
    const std::lock_guard block_room_changes{mutability};
    current_screen_size = size;

    std::visit([size](auto& room)
    {
        if constexpr(idle_has_method(idle_remove_cvr(room), on_resize))
        {
            room.on_resize(size);
        }
    }, current_variant);
}

void controller::draw_frame(const graphics::core& gl) noexcept
{
    if (haiku.has_crashed())
    {
        gl.prog.text.use();
        gl.prog.text.set_color({1, 1, 1, .9f});

        draw_text<text_align::center, text_align::center>(*gl.fonts.regular, gl.prog.text,
                haiku.get_string(), gl.draw_size / 2.f, 32);
    }
    else
    {
        gl.prog.normal.use();

        const std::lock_guard block_room_changes{mutability};

        std::visit([&gl](auto& room)
        {
            if constexpr(idle_has_method(idle_remove_cvr(room), draw))
            {
                room.draw(gl);
            }
        }, current_variant);
    }
}

void controller::awaken(const std::chrono::steady_clock::time_point clock) noexcept
{
    using namespace std::chrono_literals;
    constexpr auto skip_a_beat = std::chrono::duration_cast<std::chrono::microseconds>(1.5s) / application_frames_per_second;
    worker.stop();

    if (!haiku.has_crashed())
    {
        worker.start([this](std::chrono::steady_clock::time_point step)
            {
                LOGD("Room service [🈺]");

                pointer_keeper pointer;

                while (worker.is_active())
                {
#ifdef IDLE_COMPILE_FPS_COUNTERS
                    const auto before_waiting = std::chrono::high_resolution_clock::now();
#endif
                    step = wait_one_frame(step);
#ifdef IDLE_COMPILE_FPS_COUNTERS
                    teller.count_fps(before_waiting);
#endif

                    if (const auto maybe_action = do_step(pointer.get()))
                    {
                        std::visit([this](auto& action)
                            {
                                using type = idle_remove_cvr(action);

                                if constexpr (std::is_same_v<type, std::string_view>)
                                {
                                    sleep();
                                    haiku.crash(action);
                                }
                                else if constexpr (is_hotel_room<type>::value)
                                {
                                    next_variant.rooms.emplace(door<typename type::opened_type>{});
                                }
                                else
                                {
                                    sleep();
                                    haiku.crash("Unknown action requested");
                                }
                            },
                            *maybe_action);
                    }
                    pointer.advance(cached_cursor.load(std::memory_order_relaxed));
                }

                LOGD("Room service [💤]");
            },
            clock + skip_a_beat);
    }
}

namespace
{
template<typename T>
constexpr char room_label[] = "UNNAMED";

template<>
constexpr char room_label<hotel::landing::room>[] = "LANDING";

template<>
constexpr char room_label<hotel::stage::room>[] = "STAGE";

#ifdef IDLE_COMPILE_GALLERY
template<>
constexpr char room_label<hotel::model::room>[] = "MODEL";
#endif

}  // namespace

std::optional<hotel::keyring::variant> controller::do_step(const pointer_wrapper& cur) noexcept
{
    if (next_variant.rooms)
    {
        const std::lock_guard block_drawing_and_resizing{mutability};

        std::visit([this] (const auto& gate)
            {
                using T = typename idle_remove_cvr(gate)::opened_type;
                LOGI("Switching context to: %s", room_label<T>);

                static_assert(std::is_constructible_v<T>);
                gate.open(current_variant);

                if constexpr(idle_has_method(T, on_resize))
                {
                    std::get<T>(current_variant).on_resize(current_screen_size);
                }
            },
            *next_variant.rooms);

        next_variant.rooms.reset();
        return {};
    }

    return std::visit([&cur] (auto& room) -> std::optional<hotel::keyring::variant>
        {
            if constexpr(idle_has_method(idle_remove_cvr(room), step))
            {
                return room.step(cur);
            }
            else
            {
                return {};
            }
        },
        current_variant);
}

}  // namespace idle

