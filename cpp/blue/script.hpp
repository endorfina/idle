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
#include <utf8.hpp>
#include <vector>
#include <string>
#include <string_view>
#include <map>
#include <unordered_map>
#include <memory>
#include <functional>

#include "types.hpp"
#include "builtin.hpp"
#include "what.hpp"
#include "exception.hpp"

namespace blue
{
    template<class T, class Var, typename
        = std::enable_if_t<std::is_same_v<std::remove_cv_t<std::remove_reference_t<Var>>, variable_t>>>
    static T cast(Var && var) {
        return std::visit([](auto &&v) -> T {
            using V = std::remove_cv_t<std::remove_reference_t<decltype(v)>>;
            using TT = std::remove_cv_t<std::remove_reference_t<T>>;
            static_assert(!std::is_reference_v<T> || !std::is_const_v<TT>);
            if constexpr (std::is_same_v<V, TT>) {
                return v;
            }
            else if constexpr (std::is_same_v<V, reference>) {
                if (v.variant_data)
                    return cast<T>(*reinterpret_cast<variable_t*>(v.variant_data));
                else if (v.container_data && what<TT>::value == *v.container_data)
                    return *reinterpret_cast<TT*>(v.container_data + 1);
                std::abort();
            }
            else if constexpr (std::is_convertible_v<V, T>) {
                return T(v);
            }
            else if constexpr (std::is_constructible_v<T, V>) {
                return T(v);
            }
            else std::abort();
        }, var);
    }

    struct parser
    {
        namespace_t names;
        bool get_lines = false;
        bool failed = false;
        std::string error_string;

    private:
        return_t parse(utf8x::translator<char>& _src, bool evaluate = true);

    public:
        parser& operator<<(utf8x::translator<char> src);

        const variable_t& operator[](std::string_view key) const;

        variable_t& operator[](std::string_view key);

        bool has_defined(std::string_view key) const;

        void report_failure(const char * title, std::string_view code);
    };


    void print(const variable_t &var, void (*callback)(void*, std::string_view), void* const param);

    std::string print_to_str(const variable_t &var);
}
