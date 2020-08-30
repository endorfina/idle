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
#include <string_view>
#include <iterator>
#include <algorithm>
#include <memory>
#include <iostream>
#include <cstring>

/* tbb is not yet ready
#if __has_include(<execution>)
    #include <execution>
    #define BLUE_FOR_EACH(...) std::for_each(std::execution::par, __VA_ARGS__)
    #define BLUE_FIND(...) std::find_if(std::execution::par, __VA_ARGS__)
#else */
    #define BLUE_FOR_EACH(...) std::for_each(__VA_ARGS__)
    #define BLUE_FIND(...) std::find_if(__VA_ARGS__)
/* #endif */

#include "builtin.hpp"
#include "what.hpp"
#include "exception.hpp"

namespace blue
{
namespace
{

void destruct_memory(Types* const ptr)
{
    switch(*ptr)
    {
        case Types::Dictionary:
            std::destroy_at(reinterpret_cast<dictionary*>(ptr + 1));
            break;
        case Types::List:
            std::destroy_at(reinterpret_cast<list*>(ptr + 1));
            break;
        default:
            break;
    }
}

size_t yank_size(const variable_t& var)
{
    return std::visit([](const auto& var) {
            return sizeof(std::remove_reference_t<decltype(var)>);
    }, var);
}

template<class Var>
ptrdiff_t construct_memory_in_container(std::vector<char>& out, Var&& var)
{
    return static_cast<ptrdiff_t>(std::visit([&out](auto&&var) {
        using V = std::remove_reference_t<decltype(var)>;
        using VV = std::remove_cv_t<V>;
        const auto oldsize = out.size();
        out.resize(oldsize + sizeof(unsigned short) + sizeof(Types) + sizeof(VV));
        *reinterpret_cast<unsigned short*>(out.data() + oldsize) = sizeof(Types) + sizeof(VV);
        const auto ptr = reinterpret_cast<Types*>(out.data() + (oldsize + sizeof(unsigned short)));
        *ptr = what<VV>::value;
        ::new(reinterpret_cast<void*>(ptr + 1)) VV(std::forward<V>(var));
        return oldsize + sizeof(unsigned short);
    }, std::forward<Var>(var)));
}

template<class Var>
ptrdiff_t construct_memory_in_container(std::vector<char>& out, const string_t name, Var&& var)
{
    return static_cast<ptrdiff_t>(std::visit([&out, name](auto&&var) {
        using V = std::remove_reference_t<decltype(var)>;
        using VV = std::remove_cv_t<V>;
        const auto oldsize = out.size();
        out.resize(oldsize + sizeof(unsigned short) + sizeof(string_t) + sizeof(Types) + sizeof(VV));
        *reinterpret_cast<unsigned short*>(out.data() + oldsize) = sizeof(string_t) + sizeof(Types) + sizeof(VV);
        *reinterpret_cast<string_t*>(out.data() + oldsize + sizeof(unsigned short)) = name;
        const auto ptr = reinterpret_cast<Types*>(out.data() + (oldsize + sizeof(unsigned short) + sizeof(string_t)));
        *ptr = what<VV>::value;
        ::new(reinterpret_cast<void*>(ptr + 1)) VV(std::forward<V>(var));
        return oldsize + sizeof(unsigned short) + sizeof(string_t);
    }, std::forward<Var>(var)));
}

template<typename T>
void _copy_construct_memory(void* const dest, const void* const source)
{
    ::new(dest) T(*reinterpret_cast<const T*>(source));
}

template<typename T>
void _move_construct_memory(T* const dest, T* const source)
{
    ::new(dest) T(std::move(*source));
}

template<class Var>
void construct_variant(Types* const ptr, Var&& var)
{
    std::visit([ptr](auto&& var) {
        using V = std::remove_reference_t<decltype(var)>;
        using VV = std::remove_cv_t<V>;
        *ptr = what<VV>::value;
        ::new(reinterpret_cast<void*>(ptr + 1)) VV(std::forward<V>(var));
    }, std::forward<Var>(var));
}

void copy_memory(Types* dest, const Types* source)
{
    // long, float, std::string_view, bool, Types, function, dictionary, list, object, reference
    switch(*dest++ = *source++)
    {
        case Types::Dictionary:
            _copy_construct_memory<dictionary>(dest, source); break;
        case Types::List:
            _copy_construct_memory<list>(dest, source); break;
        default:
            break;
    }
}

// void move_memory(Types* dest, Types* source)
// {
//     // long, float, std::string_view, bool, Types, function, dictionary, list, object, reference
//     switch(*dest++ = *source++)
//     {
//         case Types::Dictionary:
//             _move_construct_memory<dictionary>(reinterpret_cast<dictionary *>(dest), reinterpret_cast<dictionary *>(source)); break;
//         case Types::List:
//             _move_construct_memory<list>(reinterpret_cast<list *>(dest), reinterpret_cast<list *>(source)); break;
//         default:
//             break;
//     }
// }

variable_t translate_memory(const Types* ptr)
{
    // long, float, std::string_view, bool, Types, function, dictionary, list, object, reference
    switch(*ptr)
    {
        case Types::Integer:
            return { *reinterpret_cast<const int_t*>(++ptr) };
        case Types::Float:
            return { *reinterpret_cast<const float_t*>(++ptr) };
        case Types::String:
            return { *reinterpret_cast<const string_t*>(++ptr) };
        case Types::Bool:
            return { *reinterpret_cast<const bool_t*>(++ptr) };
        case Types::Dictionary:
        case Types::List:
            return { reference{nullptr, (Types*)ptr} };
        case Types::Reference:
            return { *reinterpret_cast<const reference*>(++ptr) };
        default:
            std::abort();
    }
}


}  // namespace

Types yield_type(const variable_t& var)
{
    const variable_t* target = &var;
    if (auto ref = std::get_if<reference>(&var)) {
        if (ref->variant_data)
            target = reinterpret_cast<variable_t*>(ref->variant_data);
        else
            if (ref->container_data)
                return *ref->container_data;
        else
            std::abort();
    }
    return std::visit([](auto &&v){
        return blue::what<std::remove_cv_t<std::remove_reference_t<decltype(v)>>>::value;
    }, *target);
}

std::string_view print_type(Types t)
{
    switch(t)
    {
        case Types::Integer:
            return "Integer";
        case Types::Float:
            return "Float";
        case Types::String:
            return "String";
        case Types::Bool:
            return "Bool";
        case Types::Function:
            return "Function";
        case Types::Dictionary:
            return "Dictionary";
        case Types::List:
            return "List";
        case Types::Reference:
            return "Reference";
        default:
            return "<unknown-type>";
    }
}

size_t list::size() const
{
    return table.size();
}

void list::push_back(const variable_t& var)
{
    table.push_back(construct_memory_in_container(data, var));
}

void list::push_back(variable_t&& var)
{
    table.push_back(construct_memory_in_container(data, std::move(var)));
}

Types* list::operator[](const size_t index)
{
    return const_cast<Types*>(const_cast<const list*>(this)->operator[](index));
}

const Types* list::operator[](const size_t index) const
{
    return reinterpret_cast<const Types*>(data.data() + table[index]);
}

list::list(const list& other) : data(other.data.size()), table(other.table)
{
    memcpy(data.data(), other.data.data(), data.size());
    BLUE_FOR_EACH(table.begin(), table.end(), [ptr = data.data(), sptr = other.data.data()](const auto offset) {
        copy_memory(reinterpret_cast<Types*>(ptr + offset), reinterpret_cast<const Types*>(sptr + offset));
    });
}

list& list::operator=(const list& other)
{
    _destruct_();
    table = other.table;
    data.resize(other.data.size());

    memcpy(data.data(), other.data.data(), data.size());
    BLUE_FOR_EACH(table.begin(), table.end(), [ptr = data.data(), sptr = other.data.data()](const auto offset) {
        copy_memory(reinterpret_cast<Types*>(ptr + offset), reinterpret_cast<const Types*>(sptr + offset));
    });
    return *this;
}

list::~list()
{
    _destruct_();
}

void list::_destruct_()
{
    BLUE_FOR_EACH(begin(), end(), [](auto& offset)
    {
        destruct_memory(&offset);
    });
}

void list::clear()
{
    _destruct_();
    data.clear();
    table.clear();
}

list::iterator list::begin()
{
    return { data.data() };
}

list::iterator list::end()
{
    return { data.data() + data.size() };
}

list::const_iterator list::begin() const
{
    return { data.data() };
}

list::const_iterator list::end() const
{
    return { data.data() + data.size() };
}

list::iterator::reference list::iterator::operator*() const
{
    return *reinterpret_cast<pointer>(data + sizeof(unsigned short));
}

list::iterator::pointer list::iterator::operator->() const
{
    return reinterpret_cast<pointer>(data + sizeof(unsigned short));
}

list::iterator& list::iterator::operator++()
{
    data += *reinterpret_cast<unsigned short*>(data) + sizeof(unsigned short);
    return *this;
}

bool list::iterator::operator!=(const list::iterator& other) const
{
    return data != other.data;
}

list::const_iterator::reference list::const_iterator::operator*() const
{
    return *reinterpret_cast<pointer>(data + sizeof(unsigned short));
}

list::const_iterator::pointer list::const_iterator::operator->() const
{
    return reinterpret_cast<pointer>(data + sizeof(unsigned short));
}

list::const_iterator& list::const_iterator::operator++()
{
    data += *reinterpret_cast<const unsigned short*>(data) + sizeof(unsigned short);
    return *this;
}

bool list::const_iterator::operator!=(const list::const_iterator& other) const
{
    return data != other.data;
}

size_t dictionary::size() const
{
    return size_;
}

Types* dictionary::_realign_mem_(const size_t vs, const string_t name)
{
    const auto oldused = used_;
    used_ += sizeof(unsigned short) + sizeof(string_t) + sizeof(Types) + vs;

    if (!data)
    {
        reserved_ = used_ + 80;
        data = std::make_unique<char[]>(reserved_);
    }
    else if (used_ > reserved_)
    {
        reserved_ = used_ + used_ / 2;
        auto n = std::make_unique<char[]>(reserved_);
        memcpy(n.get(), data.get(), oldused);
        /* for now, memcpy takes care of everything */

        // BLUE_FOR_EACH(begin(), iterator{data.get() + oldused}, [d = n.get(), s = data.get()](auto& it) {
        //     const ptrdiff_t offset = reinterpret_cast<const char*>(it.ptr) - s;
        //     move_memory(reinterpret_cast<Types*>(d + offset), reinterpret_cast<Types*>(s + offset));
        // });
        data = std::move(n);
    }

    *reinterpret_cast<unsigned short*>(data.get() + oldused) = sizeof(string_t) + sizeof(Types) + vs;
    *reinterpret_cast<string_t*>(data.get() + oldused + sizeof(unsigned short)) = name;
    return reinterpret_cast<Types*>(data.get() + (oldused + sizeof(unsigned short) + sizeof(string_t)));
}

void dictionary::push(string_t name, const variable_t& var)
{
    if (!!find(name))
        std::abort();
        // throw exception{"Dictionary entry defined twice", name};
    construct_variant(_realign_mem_(yank_size(var), name), var);
    ++size_;
}

void dictionary::push(string_t name, variable_t&& var)
{
    if (!!find(name))
        std::abort();
        // throw exception{"Dictionary entry defined twice", name};
    construct_variant(_realign_mem_(yank_size(var), name), std::move(var));
    ++size_;
}

const Types* dictionary::find(const string_t key) const
{
    const auto f = BLUE_FIND(begin(), end(), [key](const auto& it)
    {
        return key == it.name;
    });
    if (f != end())
        return &f->type;
    return nullptr;
}

Types* dictionary::find(const string_t key)
{
    return const_cast<Types*>(const_cast<const dictionary*>(this)->find(key));
}

return_t dictionary::operator[](string_t name)
{
    if (const auto ptr = find(name))
        return { translate_memory(ptr) };
    return {};
}

dictionary::dictionary(const dictionary& other)
    : data(std::make_unique<char[]>(other.used_)), size_(other.size_), reserved_(other.used_), used_(other.used_)
{
    memcpy(data.get(), other.data.get(), used_);
    BLUE_FOR_EACH(other.begin(), other.end(), [ptr = data.get(), sptr = other.data.get()](const auto& it) {
        const ptrdiff_t offset = reinterpret_cast<const char*>(&it.type) - sptr;
        copy_memory(reinterpret_cast<Types*>(ptr + offset), reinterpret_cast<const Types*>(sptr + offset));
    });
}

dictionary& dictionary::operator=(const dictionary& other)
{
    _destruct_();
    data = std::make_unique<char[]>(other.used_);
    size_ = other.size_;
    used_ = reserved_ = other.used_;

    memcpy(data.get(), other.data.get(), used_);
    BLUE_FOR_EACH(other.begin(), other.end(), [ptr = data.get(), sptr = other.data.get()](const auto& it) {
        const ptrdiff_t offset = reinterpret_cast<const char*>(&it.type) - sptr;
        copy_memory(reinterpret_cast<Types*>(ptr + offset), reinterpret_cast<const Types*>(sptr + offset));
    });
    return *this;
}

dictionary::~dictionary()
{
    _destruct_();
}

void dictionary::_destruct_()
{
    if (data) {
        BLUE_FOR_EACH(begin(), end(), [](auto& it)
        {
            destruct_memory(&it.type);
        });
    }
}

void dictionary::clear()
{
    _destruct_();
    used_ = size_ = 0;
}

dictionary::iterator dictionary::begin()
{
    return { data.get() };
}

dictionary::iterator dictionary::end()
{
    return { data.get() + used_ };
}

dictionary::const_iterator dictionary::begin() const
{
    return { data.get() };
}

dictionary::const_iterator dictionary::end() const
{
    return { data.get() + used_ };
}

dictionary::iterator::iterator(char * d)
    : data(d)
{}

dictionary::iterator::reference dictionary::iterator::operator*() const
{
    return *reinterpret_cast<header*>(data + sizeof(unsigned short));
}

dictionary::iterator::pointer dictionary::iterator::operator->() const
{
    return reinterpret_cast<header*>(data + sizeof(unsigned short));
}

dictionary::iterator& dictionary::iterator::operator++()
{
    data += *reinterpret_cast<unsigned short*>(data) + sizeof(unsigned short);
    return *this;
}

bool dictionary::iterator::operator!=(const dictionary::iterator& other) const
{
    return data != other.data;
}

dictionary::const_iterator::const_iterator(const char * d)
    : data(d)
{}

dictionary::const_iterator::reference dictionary::const_iterator::operator*() const
{
    return *reinterpret_cast<const header*>(data + sizeof(unsigned short));
}

dictionary::const_iterator::pointer dictionary::const_iterator::operator->() const
{
    return reinterpret_cast<const header*>(data + sizeof(unsigned short));
}

dictionary::const_iterator& dictionary::const_iterator::operator++()
{
    data += *reinterpret_cast<const unsigned short*>(data) + sizeof(unsigned short);
    return *this;
}

bool dictionary::const_iterator::operator!=(const dictionary::const_iterator& other) const
{
    return data != other.data;
}

std::vector<std::string_view> dictionary::keys() const
{
    std::vector<std::string_view> out(size());
    std::transform(begin(), end(), out.begin(),
    [](const auto& a) {
        return a.name;
    });
    return out;
}

}  // namespace blue
