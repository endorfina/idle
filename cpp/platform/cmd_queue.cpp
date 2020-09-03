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

#include "cmd_queue.hpp"

namespace platform
{

bool command_queue_t::is_full() const
{
    return count >= raw_queue.size();
}

const command * command_queue_t::begin() const
{
    return &raw_queue[0];
}

const command * command_queue_t::end() const
{
    return begin() + count;
}

void command_queue_t::insert(const command new_cmd)
{
    raw_queue[count++] = new_cmd;
}

void command_queue_t::clear()
{
    count = 0;
}

unsigned command_queue_t::size() const
{
    return count;
}

resize_request_t::resize_request_t(const unsigned width, const unsigned height)
    : w(width), h(height)
{
}

}  // namespace platform
