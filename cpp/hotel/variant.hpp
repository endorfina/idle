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
#include <variant>
#include "room_landing.hpp"
#include "room_stage.hpp"
#include "room_model.hpp"

namespace idle::hotel
{

namespace meta
{

template<typename T>
struct room_strip_error {};

template<typename ErrorType, typename...Rooms>
struct room_strip_error<
        std::variant<ErrorType, keyring::somewhere_else<Rooms>...>
    >
{
    using type = std::variant<Rooms...>;
};

}  // namespace meta

using rooms = typename meta::room_strip_error<keyring::variant>::type;

}
