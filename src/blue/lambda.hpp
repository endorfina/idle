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
#error "Deprecated. Do not use"
#include <tuple>
#include <type_traits>
#include <vector>
#include <unordered_map>
#include <string_view>

#include "builtin.hpp"

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
                throw exception{"Failed to dereference."};
            }
            else if constexpr (std::is_convertible_v<V, T>) {
                return T(v);
            }
            else if constexpr (std::is_constructible_v<T, V>) {
                return T(v);
            }
            else throw exception{"Bad blue::cast from " + std::string(print_type(what<V>::value)) + " to " + std::string(print_type(what<TT>::value))};
        }, var);
    }

    template<class Lambda, class...Args>
    class lambda_holder : public callable {
        using tuple_t = std::tuple<Args...>;
#ifndef __ANDROID__
        using result_t = std::invoke_result_t<Lambda, Args...>;
#else
        using result_t = decltype(std::apply<Lambda, tuple_t>(std::declval<Lambda>(), std::declval<tuple_t>()));
#endif
        Lambda _l;

        /* decltype(auto) can perfectly forward an lvalue ref, while auto forwards a value */
        template<size_t I>
        static decltype(auto) _set_(const variable_t *in) {
            return cast<std::tuple_element_t<I, tuple_t>>(in[I]);
        }

        template<size_t... Is>
        static tuple_t _for_each_(std::index_sequence<Is...>, args_t in) {
            return tuple_t { _set_<Is>(in.get())... };
        }
    public:
        lambda_holder(Lambda&&_l_) : _l(_l_) {}

        return_t operator()(args_t a) override {
            tuple_t t = _for_each_(std::make_index_sequence<sizeof...(Args)>{}, std::move(a));
            if constexpr (std::is_void_v<result_t>) {
                std::apply(_l, std::move(t));
                return {};
            }
            else
                return { std::apply(_l, std::move(t)) };
        }
    };

    class callback_extern {
        std::unordered_map<std::string_view, std::pair<unsigned, std::unique_ptr<callable>>> _db;
    public:
        function operator()(std::string_view name);

        template<class... Args, class L>
        void insert(std::string_view name, L&&t) {
            auto &[count, ptr] = _db[name];
            count = sizeof...(Args);
            ptr.reset(new lambda_holder<L, Args...>(std::forward<L>(t)));
        }
    };
}
