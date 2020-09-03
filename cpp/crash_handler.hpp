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

#include <string_view>
#include <atomic>

namespace idle
{

class crash_handler
{
    std::string_view error_string;
    std::atomic_bool crashed{false};

public:
    std::string_view get_string() const noexcept;

    bool has_crashed() const noexcept;

    void crash(std::string_view haiku) noexcept;
};

}  // namespace idle
