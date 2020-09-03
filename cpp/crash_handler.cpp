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

#include "crash_handler.hpp"

namespace idle
{

void crash_handler::crash(std::string_view str) noexcept
{
    error_string = str;
    crashed.store(true, std::memory_order_release);
}

bool crash_handler::has_crashed() const noexcept
{
    return crashed.load(std::memory_order_acquire);
}

std::string_view crash_handler::get_string() const noexcept
{
    return error_string;
}

}  // namespace idle

