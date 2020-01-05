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
#include <vector>
#include <array>
#include <set>
#include <cstddef>
#include <limits>


namespace cells
{

template<typename index_type, typename value_type>
struct cell
{
    index_type skip = std::numeric_limits<index_type>::max();
    std::byte data[sizeof(value_type)];

    const value_type& get() const
    {
        return *std::launder(reinterpret_cast<const value_type*>(data));
    }

    value_type& get()
    {
        return *std::launder(reinterpret_cast<value_type*>(data));
    }

    void destroy()
    {
        if (!skip) std::destroy_at(&get());
        skip = 0;
    }

    ~cell()
    {
        destroy();
    }

    template<typename...Args>
    void emplace(Args&&...args)
    {
        destroy();
        ::new(static_cast<void*>(&get())) value_type(std::forward<Args>(args)...);
    }
};

template<typename value_type>
struct iterator
{
    value_type* ptr;

    void skip()
    {
        if (!!ptr->skip)
            ptr += ptr->skip;
    }

    auto& operator*() const
    {
        skip();
        return ptr->get();
    }

    auto& operator*()
    {
        skip();
        return ptr->get();
    }

    auto operator->() const
    {
        skip();
        return &ptr->get();
    }

    auto operator->()
    {
        skip();
        return &ptr->get();
    }

    iterator& operator++()
    {
        ++ptr;
        return *this;
    }

    iterator operator++(int)
    {
        iterator copy{*this};
        this->operator++();
        return copy;
    }
};

template<typename value_type>
bool operator!=(const iterator<value_type>& lhs, const iterator<value_type>& rhs)
{
    return lhs.ptr != rhs.ptr;
}

template<typename index_type, typename value_type>
struct colony
{
    using cell_type = cell<index_type, value_type>;
    using container_type = std::vector<cell_type>;

    unsigned int count = 0;
    container_type data;

    template<typename...Args>
    value_type& emplace(Args&&...args)
    {
        if (count++ < data.size())
        {
            auto find = begin().ptr;
            while (!find->skip)
                ++find;
            find->emplace(std::forward<Args>(args)...);
            return find->get();
        }
        else
        {
            auto& obj = data.emplace_back();
            obj.emplace(std::forward<Args>(args)...);
            return obj.get();
        }
    }

    void remove(const iterator<cell_type> removed)
    {
        if (!count || !!removed.ptr->skip) return;
        --count;
        removed.ptr->destroy();

        const auto b = begin().ptr, e = end().ptr;
        index_type skip_val = 1;

        if (const auto next = removed.ptr + 1; next != e)
            skip_val += next->skip;

        for (auto prev = removed.ptr; prev-- != b && !!prev->skip;)
            prev->skip += skip_val;

        removed.ptr->skip = skip_val;
    }

    iterator<const cell_type> begin() const
    {
        return { &data[0] };
    }

    iterator<const cell_type> end() const
    {
        return { &data[0] + data.size() };
    }

    iterator<cell_type> begin()
    {
        return { &data[0] };
    }

    iterator<cell_type> end()
    {
        return { &data[0] + data.size() };
    }
};

}  // namespace cells

