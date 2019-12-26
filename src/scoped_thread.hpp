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
#include <thread>
#include <atomic>

namespace idle
{
class scoped_thread
{
    std::atomic_bool destruction_flag;
    std::thread thrd;
public:
    template<class Functor>
    explicit scoped_thread(Functor && f)
        : destruction_flag(false),
        thrd([f = std::forward<Functor>(f), this]{ while(!destruction_flag && f()); })
    {}

    ~scoped_thread();
};
}
