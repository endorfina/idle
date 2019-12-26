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

namespace violet
{
    template<typename A, unsigned int ChunkSize = 64u>
    class colony {
    public:
        using value_type = A;

    private:
        using pos_t =           uint_fast16_t;
        using memory_t =        std::array<std::byte, sizeof(value_type) / sizeof(std::byte)>;
        using pair_t =          std::pair<pos_t, memory_t>;
        using set_t =           std::set<pos_t>;
        using list_t =          std::vector<std::pair<std::array<pair_t, ChunkSize>, set_t>>;

        list_t _data;
        size_t _inner_size = 0;

    public:
        class iterator {
            pos_t _chunk_pos = 0;
            using internal_iter_t = typename list_t::iterator;
            internal_iter_t _l_it, _l_end;

        public:
            iterator(internal_iter_t _b, internal_iter_t _e)
                : _l_it(_b), _l_end(_e) {}

            const value_type* operator->() const { return reinterpret_cast<const value_type*>(_l_it->first[_chunk_pos].second.data()); }

            value_type* operator->() { return reinterpret_cast<value_type*>(_l_it->first[_chunk_pos].second.data()); }

            const value_type& operator*() const& { return *reinterpret_cast<const value_type*>(_l_it->first[_chunk_pos].second.data()); }

            value_type& operator*() & { return *reinterpret_cast<value_type*>(_l_it->first[_chunk_pos].second.data()); }

            const value_type&& operator*() const&& { return reinterpret_cast<const value_type&&>(*_l_it->first[_chunk_pos].second.data()); }

            value_type&& operator*() && { return reinterpret_cast<value_type&&>(*_l_it->first[_chunk_pos].second.data()); }

            iterator &operator++() {
                ++_chunk_pos;
                while (true) {
                    if (_chunk_pos < pos_t(ChunkSize)) {
                        if (const auto skip = _l_it->first[_chunk_pos].first; skip > 0)
                            _chunk_pos += skip;
                        else
                            break;
                    }
                    else {
                        _chunk_pos = 0;
                        ++_l_it;
                        if (is_at_end())
                            break;
                    }
                }
                return *this;
            }

            iterator operator++(int) {
                iterator copy{*this};
                this->operator++();
                return copy;
            }

            bool is_at_end() const { return _l_it == _l_end; }

            void excuse_yourself() {
                auto skip_val = _l_it->first[_chunk_pos].first = (
                    _chunk_pos + 1 < pos_t(ChunkSize)
                    ? _l_it->first[_chunk_pos + 1].first + 1
                    : 1);
                _l_it->second.emplace(_chunk_pos);
                if constexpr (!std::is_trivially_destructible_v<value_type>) {
                    std::destroy_at(&_l_it->first[_chunk_pos].second);
                }
                for (auto i = _chunk_pos; i > 0 && !!_l_it->first[--i].first;)
                {
                    _l_it->first[i].first = --skip_val;
                }
            }

            struct end_sentinel_ {};

            friend bool operator!=(const iterator &_lhs, const iterator &_rhs) {
                return _lhs._l_it != _rhs._l_it || _lhs._chunk_pos != _rhs._chunk_pos;
            }
            friend bool operator!=(end_sentinel_ _es, const iterator &_it) {
                return !_it.is_at_end();
            }
            friend bool operator!=(const iterator &_it, end_sentinel_ _es) {
                return !_it.is_at_end();
            }
        };

        bool empty() const { return _inner_size == 0; }
        auto size() const { return _inner_size; }

    private:
        void _destroy_everything() {
            if constexpr (!std::is_trivially_destructible_v<value_type>) {
                for (auto& it : *this) {
                    std::destroy_at(&it);
                }
            }
        }

    public:
        void clear() {
            _destroy_everything();
            _data.clear();
            _inner_size = 0;
        }

        ~colony() {
            _destroy_everything();
        }

        typename iterator::end_sentinel_ end() { return {}; }

        iterator begin() { return { _data.begin(), _data.end() }; }

        iterator end_iterator() { return { _data.end(), _data.end() }; }

        template<class... Args>
        value_type &emplace(Args&&... args)
        {
            for (auto& [chunk, set] : _data) {
                if (set.empty())
                    continue;
                const auto empty = set.begin();
                auto& it = chunk[*empty];
                set.erase(empty);
                ++_inner_size;

                it.first = 0;
                return *::new(static_cast<void*>(&(it.second))) value_type(std::forward<Args>(args)...);
            }

            auto& [array, new_set] = _data.emplace_back();
            auto ptr = array.data();
            for (pos_t i = 1; i < pos_t(ChunkSize); ++i)
            {
                ptr[i].first = pos_t(ChunkSize) - i;
                new_set.emplace(i);
            }
            ++_inner_size;
            return *::new(static_cast<void*>(&(ptr->second))) value_type(std::forward<Args>(args)...);
        }

        void erase(iterator & it) {
            it.excuse_yourself();
            ++it;
            if (_inner_size)
                --_inner_size;
        }
    };
}

























namespace violet
{

namespace cells
{

template<size_t N>
struct cell
{
    uint_fast16_t skip;
    std::array<std::byte, N> data;
};

template<typename A>
using cell_t = cell<sizeof(A)>;

template<typename A, size_t N>
using array_t = std::array<cell_t<A>, N>;

}  // namespace cells

template<typename...Types>
struct colony
{
    static_assert(sizeof...(Types) > 0);
    constexpr static size_t full_element_size = (0 + ... + sizeof(Types));

    static_assert(full_element_size > 0);
    constexpr static size_t array_length = 30000 / full_element_size + 16;

    using tuple_t = std::tuple<cells::array_t<Types, array_length>...>;
    tuple_t tuple;

    constexpr colony()
    {
        foreach<0>([] (auto& cells) {
            for (auto& c : cells) {
                c.skip = c.size();
                c.fill(0);
            }
        });
    }

private:
    template<size_t Index, typename Callable>
    constexpr inline void foreach(Callable&& call)
    {
        call(get<Index>(tuple));
        if constexpr (Index + 1 < sizeof...(Types))
            foreach<Index + 1>(std::forward<Callable>(call));
    }

public:
    template<size_t Index>
    decltype(auto) get(const size_t i)
    {
        auto &cell = get<Index>(tuple)[i];
    }
};

}  // namespace violet


