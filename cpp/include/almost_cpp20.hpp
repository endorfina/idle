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

#if !(__cpp_constinit)
#define constinit
#endif

#if __cpp_concepts

#define idle_check_method_boilerplate(methodname)
#define idle_has_method(objectname, methodname) requires { &objectname::methodname; }

#define idle_check_member_boilerplate(membername)
#define idle_has_member(objectname, membername) requires { &objectname::membername; }

#define idle_weak_requires(x) requires (x)

#else

#define idle_check_method_boilerplate(methodname) template<typename T, typename = void>\
    struct has_##methodname##_method : std::false_type {};\
    template<typename T>\
    struct has_##methodname##_method<\
        T,\
        std::enable_if_t<\
            std::is_member_function_pointer_v<decltype(&T::methodname)>\
        >\
    > : public std::true_type {}

#define idle_has_method(objectname, methodname) has_##methodname##_method<objectname>::value

#define idle_check_member_boilerplate(membername) template<typename T, typename = void>\
    struct has_##membername##_member : std::false_type {};\
    template<typename T>\
    struct has_##membername##_member<\
        T,\
        std::enable_if_t<\
            std::is_member_function_pointer_v<decltype(&T::membername)>\
        >\
    > : public std::true_type {}

#define idle_has_member(objectname, membername) has_##membername##_member<objectname>::value

#define idle_weak_requires(x)

#endif  // __cpp_concepts

