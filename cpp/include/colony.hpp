/*
    Copyright © 2020 endorfina <dev.endorfina@outlook.com>

    This file is part of Violet.

    Violet is free software: you can study it, redistribute it
    and/or modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation, either version 3 of
    the License, or (at your option) any later version.

    Violet is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Violet.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once
#include <utility>  // pair
#include <memory>
#include <set>
#include <cstddef>
#include <limits>
#include <vector>
#include <deque>

namespace cells
{

template<typename index_t, typename value_t>
struct colony
{
    using index_type = index_t;
    using value_type = value_t;

    struct cell
    {
        index_type skip = 0;

    private:
        std::byte data[sizeof(value_type)];

        template<typename...Args>
        void emplace(Args&&...args) noexcept
        {
            ::new(static_cast<void*>(&ref())) value_type(std::forward<Args>(args)...);
        }

    public:
        const value_type& ref() const noexcept
        {
            return *std::launder(reinterpret_cast<const value_type*>(data));
        }

        value_type& ref() noexcept
        {
            return *std::launder(reinterpret_cast<value_type*>(data));
        }

        template<typename...Args>
        cell(Args&&...args)
        {
            emplace(std::forward<Args>(args)...);
        }

        void destroy(const index_type new_skip) noexcept
        {
            if (skip == 0)
            {
                std::destroy_at(&ref());
            }
            skip = new_skip;
        }

        ~cell() noexcept
        {
            destroy(0);
        }

        template<typename...Args>
        void reset(Args&&...args) noexcept
        {
            destroy(0);
            emplace(std::forward<Args>(args)...);
        }
    };

    using container_type = std::deque<cell>;
    using cell_iterator_type = typename container_type::iterator;

    class iterator
    {
        friend struct colony;
        cell_iterator_type de_iter;

        iterator(const cell_iterator_type& iter) noexcept
            : de_iter{iter}
        {}

        void advance() noexcept
        {
            if (de_iter->skip > 0)
            {
                de_iter += de_iter->skip;
            }
        }

    public:
        index_type count = 0;

        value_type& operator*() const noexcept
        {
            advance();
            return de_iter->ref();
        }

        value_type& operator*() noexcept
        {
            advance();
            return de_iter->ref();
        }

        const value_type* operator->() const noexcept
        {
            advance();
            return &de_iter->ref();
        }

        value_type* operator->() noexcept
        {
            advance();
            return &de_iter->ref();
        }

        iterator& operator++() noexcept
        {
            ++de_iter;
            return *this;
        }

        iterator operator++(int) noexcept
        {
            iterator copy{*this};
            this->operator++();
            return copy;
        }

        bool operator!=(const cell_iterator_type& rhs) const noexcept
        {
            return de_iter != rhs;
        }

        bool operator!=(const iterator& rhs) const noexcept
        {
            return de_iter != rhs.de_iter;
        }
    };

private:
    index_type count = 0;
    container_type deque;

public:
    index_type size() const noexcept
    {
        return count;
    }

    template<typename...Args>
    value_type& emplace(Args&&...args) noexcept
    {
        if (count++ < deque.size())
        {
            auto find = deque.begin();
            while (!find->skip) // assume empty space exists
                ++find;

            find->reset(std::forward<Args>(args)...);
            return find->ref();
        }
        else
        {
            auto& obj = deque.emplace_back(std::forward<Args>(args)...);
            return obj.ref();
        }
    }

    void remove(const cell_iterator_type removed) noexcept
    {
        if (!count || !!removed->skip) return;
        --count;

        index_type new_skip = 1;

        if (const auto next = removed + 1; next != deque.end())
        {
            new_skip += next->skip;
        }

        while (removed != deque.begin() && removed->skip > 0)
        {
            (removed - 1)->skip += new_skip;
        }

        removed->destroy(new_skip);
    }

    void remove(const iterator& removed) noexcept
    {
        remove(removed.de_iter);
    }

    iterator begin() const noexcept
    {
        return { deque.begin() };
    }

    cell_iterator_type end() const noexcept
    {
        return deque.end();
    }

    iterator begin() noexcept
    {
        return { deque.begin() };
    }

    cell_iterator_type end() noexcept
    {
        return deque.end();
    }
};


}  // namespace cells

