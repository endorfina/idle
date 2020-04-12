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

namespace platform
{

enum class command : uint_fast8_t
{
    SaveState,
    InitWindow,
    CloseWindow,
    GainedFocus,
    LostFocus,
    GLCleanUp,
    PausePressed
};

struct command_queue_t
{
    unsigned count = 0;
    std::array<command, (sizeof(void*) * 2) / sizeof(command)> raw_queue;

    bool is_full() const;

    const command * begin() const;

    const command * end() const;

    void insert(command cmd);

    void clear();

    unsigned size() const;
};

struct resize_request_t
{
    int w, h;

    resize_request_t(int, int);
};

}  // namespace platform

