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

#include "stage_crawlers.hpp"

namespace idle::hotel::stage
{

crawler_pool::~crawler_pool() noexcept
{
    kill_worker();
}

void crawler_pool::kill_worker() noexcept
{
    worker_alive_flag = false;
    cond_variable.notify_all();

    for (auto& it : workers)
    {
        it.join();
    }
    workers.clear();
}

void crawler_pool::notify() noexcept
{
    cond_variable.notify_all();
}

}  // namespace idle::hotel::stage

