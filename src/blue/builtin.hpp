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

#include <vector>
#include "types.hpp"
#include "what.hpp"

namespace blue
{
    Types yield_type(const variable_t&);

    std::string_view print_type(Types);

    struct reference {
        void* variant_data = nullptr;
        Types* container_data = nullptr;
    };

    template<class T, class V, typename = std::enable_if_t<std::is_same_v<Types, std::remove_cv_t<V>>>>
    std::conditional_t<std::is_const_v<V>, std::add_const_t<T>, T>* get(V* ptr)
    {
        if (what<T>::value == *ptr)
            return reinterpret_cast<std::conditional_t<std::is_const_v<V>, std::add_const_t<T>, T>*>(ptr + 1);
        return nullptr;
    }

    class dictionary {
        std::unique_ptr<char[]> data;
        size_t size_ = 0, reserved_ = 0, used_ = 0;

        void _destruct_();

        Types* _realign_mem_(const size_t vs, const string_t name);

    public:
        void clear();

        size_t size() const;

        void push(string_t name, variable_t&& var);

        void push(string_t name, const variable_t& var);

        return_t operator[](string_t name);

        dictionary() = default;

        dictionary(dictionary&&) = default;

        dictionary(const dictionary&);

        ~dictionary();

        dictionary& operator=(const dictionary&);

        dictionary& operator=(dictionary&&) = default;

    private:
        const Types* find(const string_t) const;

        Types* find(const string_t);

    public:
        struct /*__attribute__ ((packed))*/ header {
            string_t name;
            Types type;

            header(const header&)=delete;
            header(header&&)=delete;
            header& operator=(const header&)=delete;
            header& operator=(header&&)=delete;
        };

        class iterator {
        public:
            using difference_type = ptrdiff_t;
            using value_type = header;
            using pointer = value_type*;
            using reference = value_type&;
            using iterator_category = std::input_iterator_tag;

        private:
            char * data;

        public:
            iterator(char * d);

            reference operator*() const;

            pointer operator->() const;

            iterator& operator++();

            bool operator!=(const iterator&) const;
        };

        iterator begin();

        iterator end();

        class const_iterator {
        public:
            using difference_type = ptrdiff_t;
            using value_type = header;
            using pointer = const value_type*;
            using reference = const value_type&;
            using iterator_category = std::input_iterator_tag;

        private:
            const char * data;

        public:
            const_iterator(const char * d);

            reference operator*() const;

            pointer operator->() const;

            const_iterator& operator++();

            bool operator!=(const const_iterator&) const;
        };

        const_iterator begin() const;

        const_iterator end() const;

        std::vector<std::string_view> keys() const;

        template<class T>
        const T* yield(const string_t name) const
        {
            if (const auto ptr = find(name))
                if (what<T>::value == *ptr)
                    return reinterpret_cast<const T*>(ptr + 1);
            return nullptr;
        }

        template<class T>
        T* yield(const string_t name)
        {
            return const_cast<T*>(const_cast<const dictionary*>(this)->yield<T>(name));
        }
    };


    class list {
        std::vector<char> data;
        std::vector<ptrdiff_t> table;

        void _destruct_();

    public:
        void clear();

        size_t size() const;

        void push_back(variable_t&& var);

        void push_back(const variable_t& var);

        Types* operator[](const size_t index);

        const Types* operator[](const size_t index) const;

        list() = default;

        list(list&&) = default;

        list(const list&);

        ~list();

        list& operator=(const list&);

        list& operator=(list&&) = default;

        class iterator {
        public:
            using difference_type = ptrdiff_t;
            using value_type = Types;
            using pointer = value_type*;
            using reference = value_type&;
            using iterator_category = std::input_iterator_tag;

        private:
            char * data;

        public:
            iterator(char * d) : data(d) {}

            reference operator*() const;

            pointer operator->() const;

            iterator& operator++();

            bool operator!=(const iterator&) const;
        };

        iterator begin();

        iterator end();

        class const_iterator {
        public:
            using difference_type = ptrdiff_t;
            using value_type = Types;
            using pointer = const value_type*;
            using reference = const value_type&;
            using iterator_category = std::input_iterator_tag;

        private:
            const char * data;

        public:
            const_iterator(const char * d) : data(d) {}

            reference operator*() const;

            pointer operator->() const;

            const_iterator& operator++();

            bool operator!=(const const_iterator&) const;
        };

        const_iterator begin() const;

        const_iterator end() const;

        template<class T>
        const T* yield(const size_t index) const
        {
            const auto ptr = reinterpret_cast<const Types*>(data.data() + table[index]);
            if (what<T>::value == *ptr)
                return reinterpret_cast<const T*>(ptr + 1);
            return nullptr;
        }

        template<class T>
        T* yield(const size_t index)
        {
            return const_cast<T*>(const_cast<const list*>(this)->yield<T>(index));
        }

        template<class T>
        std::vector<const T*> filter() const
        {
            std::vector<const T*> out;
            for (const auto addr : table)
            {
                const auto ptr = reinterpret_cast<const Types*>(data.data() + addr);
                if (what<T>::value == *ptr)
                    out.push_back(reinterpret_cast<const T*>(ptr + 1));
            }
            return out;
        }
    };

    // struct callable {
    //     virtual return_t operator()(args_t) = 0;
    //     virtual ~callable() {}
    // };

    // struct evoker {
    //     std::unordered_map<Types, function> evoked;
    //     bool has_function_handler = false;

    //     return_t call(const variable_t& arg);
    // };

    // struct function {
    //     unsigned int arg_count = 0;
    //     void* ptr = nullptr;
    //     bool is_evoker = false;

    //     function(evoker);

    //     function() = default;

    //     function(function&&) noexcept;

    //     function(const function&);

    //     ~function();

    //     function& operator=(const function&);

    //     function& operator=(function&&) noexcept;

    //     void clear();

    //     operator bool() const;

    //     return_t call(args_t) const;
    // };
}
