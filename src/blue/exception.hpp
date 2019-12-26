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
#include <string>
#include <string_view>
#include <exception>
#include <stdexcept>

namespace blue
{
    class exception : public std::runtime_error {
        static std::string _format_where_(std::string);
    public:
        template<class S1, class S2>
        exception(S1&&_what, S2&&_where)
            : std::runtime_error{static_cast<std::string>(_what) + ", at:\n" + _format_where_(static_cast<std::string>(_where))} {}

        template<class S>
        exception(S&&s)
            : std::runtime_error{std::forward<S>(s)} {}
    };
}
