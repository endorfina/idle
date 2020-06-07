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

// #include <math.hpp>
#include <variant>
#include <string_view>
#include <idle/idle_try.hpp>

namespace idle::hotel
{

namespace landing
{
struct room;
}

#ifdef IDLE_COMPILE_GALLERY
namespace model
{
struct room;
}
#endif

namespace keyring
{

template<class T>
struct somewhere_else
{
    using opened_type = T;
};

using variant = std::variant<
        somewhere_else<landing::room>,
#ifdef IDLE_COMPILE_GALLERY
        somewhere_else<model::room>,
#endif
        std::string_view
    >;

}  // namespace keyring

}  // namespace idle::hotel

