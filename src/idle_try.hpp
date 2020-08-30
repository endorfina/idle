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
#include <utility>
#include <type_traits>
#include <optional>

namespace idle::meta
{

template<typename T>
struct is_optional : std::false_type {};

template<typename T>
struct is_optional<std::optional<T>> : std::true_type {};

}  // namespace idle::meta

#define idle_remove_cvr(x) std::remove_cv_t<std::remove_reference_t<decltype(x)>>

#define idle_try(optional) static_assert(::idle::meta::is_optional<idle_remove_cvr(optional)>::value, "idle_try requires std::optional"); \
    if (auto try_result = (optional)) { return { std::move(*try_result) }; }

