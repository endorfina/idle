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
#include <string_view>

// This file is required, because each room::step()
// needs to be able to return each other room

namespace idle::hotel
{

#define idle_declare_room(name) namespace name { struct room; }

idle_declare_room(landing)

idle_declare_room(stage)

#ifdef IDLE_COMPILE_GALLERY
idle_declare_room(model)
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
        somewhere_else<stage::room>,
        std::string_view
    >;

}  // namespace keyring

#undef idle_declare_room

}  // namespace idle::hotel

