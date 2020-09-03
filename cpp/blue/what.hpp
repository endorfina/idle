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
#include "builtin.hpp"

namespace blue
{
    template<class T>
    struct what : public std::integral_constant<Types, Types::Error> {};

    template<>
    struct what<bool_t> : public std::integral_constant<Types, Types::Bool> {};

    template<>
    struct what<float_t> : public std::integral_constant<Types, Types::Float> {};

    template<>
    struct what<int_t> : public std::integral_constant<Types, Types::Integer> {};

    template<>
    struct what<string_t> : public std::integral_constant<Types, Types::String> {};

    // template<>
    // struct what<function> : public std::integral_constant<Types, Types::Function> {};

    template<>
    struct what<dictionary> : public std::integral_constant<Types, Types::Dictionary> {};

    template<>
    struct what<list> : public std::integral_constant<Types, Types::List> {};

    template<>
    struct what<reference> : public std::integral_constant<Types, Types::Reference> {};
}
