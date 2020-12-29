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

#include <type_traits>
#include <atomic>
#include <math_defines.hpp>

namespace idle
{

template<typename T>
struct relaxed
{
    using value_type = T;
    // using reference = T&;

private:
    std::atomic<T> atomic;

public:
    template<typename A,
        typename = std::enable_if_t
            <
                !std::is_same_v
                <
                    std::remove_cv_t
                    <
                        std::remove_reference_t
                        <
                            A
                        >
                    >,

                    relaxed
                >
            >
        >
    relaxed(A&& arg) noexcept : atomic(std::forward<A>(arg)) {}

    relaxed() noexcept = default;

    operator value_type() const noexcept
    {
        return atomic.load(std::memory_order_relaxed);
    }

    auto load() const noexcept
    {
        return atomic.load(std::memory_order_relaxed);
    }

    template<typename A,
        typename = std::enable_if_t
            <
                std::is_same_v
                <
                    std::remove_cv_t
                    <
                        std::remove_reference_t
                        <
                            A
                        >
                    >,

                    value_type
                >
            >
        >
    relaxed& operator=(A&& arg) noexcept
    {
        atomic.store(std::forward<A>(arg), std::memory_order_relaxed);
        return *this;
    }

    template<typename A,
        typename = std::enable_if_t
            <
                std::is_same_v
                <
                    std::remove_cv_t
                    <
                        std::remove_reference_t
                        <
                            A
                        >
                    >,

                    value_type
                >
            >
        >
    auto operator+=(A&& arg) noexcept
    {
        return atomic.fetch_add(std::forward<A>(arg), std::memory_order_relaxed);
    }

    template<typename A,
        typename = std::enable_if_t
            <
                std::is_same_v
                <
                    std::remove_cv_t
                    <
                        std::remove_reference_t
                        <
                            A
                        >
                    >,

                    value_type
                >
            >
        >
    auto operator-=(A&& arg) noexcept
    {
        return atomic.fetch_sub(std::forward<A>(arg), std::memory_order_relaxed);
    }
};

}  // namespace idle
