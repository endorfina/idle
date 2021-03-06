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

#include "service.hpp"

namespace idle::hotel
{

void room_service::set_active(const bool flag) noexcept
{
    worker_active_flag = flag;
}

void room_service::stop() noexcept
{
    set_active(false);

    if (worker_thread)
    {
        worker_thread->join();
        worker_thread.reset();
    }
}

bool room_service::is_active() const noexcept
{
    return worker_active_flag;
}

room_service::~room_service()
{
    stop();
}

}  // namespace idle::hotel

