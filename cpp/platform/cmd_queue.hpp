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
#include <stdint.h>

namespace platform
{

enum class command : uint_fast8_t
{
    save_state,
    init_window,
    close_window,
    gained_focus,
    lost_focus,
    gl_clean_up,
    pause_pressed
};

class command_queue_t
{
    unsigned count = 0;
    std::array<command, (sizeof(void*) * 2) / sizeof(command)> raw_queue;

public:
    bool is_full() const noexcept;

    const command * begin() const noexcept;

    const command * end() const noexcept;

    void insert(command cmd) noexcept;

    void clear() noexcept;

    unsigned size() const noexcept;
};

struct resize_request_t
{
    unsigned w, h;

    resize_request_t(unsigned, unsigned) noexcept;
};

}  // namespace platform

