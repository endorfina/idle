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
#include <inttypes.h>
#include <initializer_list>
#include <utility>
#include <string_view>
#include <cctype>

typedef uint_fast32_t utf8int_t;

namespace utf8x
{

template<typename T, typename = void>
struct is_view : std::false_type {};

template<typename... Ts>
struct is_view_helper {};

template<typename T>
struct is_view<
        T,
        std::conditional_t<
            false,
            is_view_helper<
                typename T::size_type,
                decltype(std::declval<T>().size()),
                decltype(std::declval<T>().begin()),
                decltype(std::declval<T>().end()),
                decltype(std::declval<T>().remove_prefix(0)),
                decltype(std::declval<T>().remove_suffix(0))
                >,
            void
            >
        > : public std::true_type {};

template<typename A>
constexpr A remove_potential_BOM(A text)
{
    std::conditional_t<is_view<A>::value, decltype(text.data()), const unsigned char *> check = nullptr;
    if constexpr (is_view<A>::value) {
        static_assert(sizeof(typename A::value_type) == 1);
        if (text.size() < 3)
            return text;
        check = text.data();
    }
    else {
        static_assert(sizeof(std::decay_t<decltype(*text)>) == 1);
        check = (const unsigned char *)&(*text);    // should take care of pointers and iterators
    }
    for (unsigned char bom : { 0xef, 0xbb, 0xbf })
        if (bom != static_cast<unsigned char>(*check++))
            return text;
    if constexpr (is_view<A>::value) {
        text.remove_prefix(3);
    }
    else {
        text += 3;
    }
    return text;
}

template <typename Char>
constexpr auto scoop(Char c) {
    return static_cast<uint8_t>(c) & 0x3f; // Scoop dat 10xxxxxx, yo
}

template <typename Char>
constexpr bool check(Char c) {
    return static_cast<uint8_t>(c) >> 6 == 0x2;
}

template <typename CharIt>
constexpr unsigned short sequence_length(CharIt it)
{
    const auto byte = static_cast<uint8_t>(*it);
    // 0xxxxxxx
    if (byte < 0x80) return 1;
    // 110xxxxx
    else if ((byte >> 5) == 0x6) return 2;
    // 1110xxxx
    else if ((byte >> 4) == 0xe) return 3;
    // 11110xxx
    else if ((byte >> 3) == 0x1e) return 4;
    // 111110xx
    else if ((byte >> 2) == 0x3e) return 5;
    // 1111110x
    else if ((byte >> 1) == 0x7e) return 6;
    return 0; // It ain't no utf8, fren. what do?
}

template <typename CharIt>
constexpr utf8int_t get_2(CharIt it) // Can be inlined?
{
    if (check(it[1]))
        return ((static_cast<utf8int_t>(*it) & 0x1f) << 6) + scoop(it[1]);
    return 0x0;
}

template <typename CharIt>
constexpr utf8int_t get_3(CharIt it)
{
    utf8int_t code_point = static_cast<utf8int_t>(*it) & 0xf;
    if (check(it[1]) && check(it[2])) {
        code_point = (code_point << 12) + (scoop(*++it) << 6);
        code_point += scoop(*++it);
    }
    return code_point;
}

template <typename CharIt>
constexpr utf8int_t get_4(CharIt it)
{
    utf8int_t code_point = static_cast<utf8int_t>(*it) & 0x7;
    if (check(it[1]) && check(it[2]) && check(it[3])) {
        code_point = (code_point << 18) + (scoop(*++it) << 12);
        code_point += scoop(*++it) << 6;
        code_point += scoop(*++it);
    }
    return code_point;
}

/* 5 and 6 byte encoding are an extension of the standard */
template <typename CharIt>
constexpr utf8int_t get_5(CharIt it)
{
    utf8int_t code_point = static_cast<utf8int_t>(*it) & 0x3;
    if (check(it[1]) && check(it[2]) && check(it[3]) && check(it[4])) {
        code_point = (code_point << 24) + (scoop(*++it) << 18);
        code_point += scoop(*++it) << 12;
        code_point += scoop(*++it) << 6;
        code_point += scoop(*++it);
    }
    return code_point;
}

template <typename CharIt>
constexpr utf8int_t get_6(CharIt it)
{
    utf8int_t code_point = static_cast<utf8int_t>(*it) & 0x1;
    if (check(it[1]) && check(it[2]) && check(it[3]) && check(it[4]) && check(it[5])) {
        code_point = (code_point << 30) + (scoop(*++it) << 24);
        code_point += scoop(*++it) << 18;
        code_point += scoop(*++it) << 12;
        code_point += scoop(*++it) << 6;
        code_point += scoop(*++it);
    }
    return code_point;
}

template <typename CharIt>
constexpr utf8int_t get_switch(CharIt it, unsigned size)
{
    switch(size) {
        case 1:
            return static_cast<uint8_t>(*it);
        case 2:
            return utf8x::get_2(it);
        case 3:
            return utf8x::get_3(it);
        case 4:
            return utf8x::get_4(it);
        case 5:
            return utf8x::get_5(it);
        case 6:
            return utf8x::get_6(it);
        default:
            //assert(false); // perhaps throw an error
            return 0;
    }
}

/* WARNING: May overflow, check sequence length first */
template <typename CharIt>
constexpr CharIt put(CharIt it,  utf8int_t code)
{
    if (code < 0x80)
        *it++ = static_cast<uint8_t>(code);
    else if (code < 0x800) {
        *it++ = static_cast<uint8_t>(code >> 6) | 0xc0;
        *it++ = (static_cast<uint8_t>(code) & 0x3f) | 0x80;
    }
    else if (code < 0x10000) {
        *it++ = static_cast<uint8_t>(code >> 12) | 0xe0;
        *it++ = (static_cast<uint8_t>(code >> 6) & 0x3f) | 0x80;
        *it++ = (static_cast<uint8_t>(code) & 0x3f) | 0x80;
    }
    else if (code < 0x200000) { // Should end at 0x110000 according to the RFC 3629
        *it++ = static_cast<uint8_t>(code >> 18) | 0xf0;
        *it++ = (static_cast<uint8_t>(code >> 12) & 0x3f) | 0x80;
        *it++ = (static_cast<uint8_t>(code >> 6) & 0x3f) | 0x80;
        *it++ = (static_cast<uint8_t>(code) & 0x3f) | 0x80;
    }
    else if (code < 0x4000000) {
        *it++ = static_cast<uint8_t>(code >> 24) | 0xf8;
        *it++ = (static_cast<uint8_t>(code >> 18) & 0x3f) | 0x80;
        *it++ = (static_cast<uint8_t>(code >> 12) & 0x3f) | 0x80;
        *it++ = (static_cast<uint8_t>(code >> 6) & 0x3f) | 0x80;
        *it++ = (static_cast<uint8_t>(code) & 0x3f) | 0x80;
    }
    else if (code < 0x80000000) {
        *it++ = static_cast<uint8_t>(code >> 30) | 0xfc;
        *it++ = (static_cast<uint8_t>(code >> 24) & 0x3f) | 0x80;
        *it++ = (static_cast<uint8_t>(code >> 18) & 0x3f) | 0x80;
        *it++ = (static_cast<uint8_t>(code >> 12) & 0x3f) | 0x80;
        *it++ = (static_cast<uint8_t>(code >> 6) & 0x3f) | 0x80;
        *it++ = (static_cast<uint8_t>(code) & 0x3f) | 0x80;
    }
    //else return put(it, 0xfffd); // The replacement character?
    // rather, don't write anything
    return it;
}

template <typename CharIt>
constexpr CharIt put_switch(CharIt it, const unsigned size, utf8int_t code)
{
    switch(size)
    {
    case 1:
        *it++ = static_cast<uint8_t>(code);
        break;

    case 2:
        *it++ = static_cast<uint8_t>(code >> 6) | 0xc0;
        *it++ = (static_cast<uint8_t>(code) & 0x3f) | 0x80;
        break;

    case 3:
        *it++ = static_cast<uint8_t>(code >> 12) | 0xe0;
        *it++ = (static_cast<uint8_t>(code >> 6) & 0x3f) | 0x80;
        *it++ = (static_cast<uint8_t>(code) & 0x3f) | 0x80;
        break;

    case 4:
        *it++ = static_cast<uint8_t>(code >> 18) | 0xf0;
        *it++ = (static_cast<uint8_t>(code >> 12) & 0x3f) | 0x80;
        *it++ = (static_cast<uint8_t>(code >> 6) & 0x3f) | 0x80;
        *it++ = (static_cast<uint8_t>(code) & 0x3f) | 0x80;
        break;

    case 5:
        *it++ = static_cast<uint8_t>(code >> 24) | 0xf8;
        *it++ = (static_cast<uint8_t>(code >> 18) & 0x3f) | 0x80;
        *it++ = (static_cast<uint8_t>(code >> 12) & 0x3f) | 0x80;
        *it++ = (static_cast<uint8_t>(code >> 6) & 0x3f) | 0x80;
        *it++ = (static_cast<uint8_t>(code) & 0x3f) | 0x80;
        break;

    case 6:
        *it++ = static_cast<uint8_t>(code >> 30) | 0xfc;
        *it++ = (static_cast<uint8_t>(code >> 24) & 0x3f) | 0x80;
        *it++ = (static_cast<uint8_t>(code >> 18) & 0x3f) | 0x80;
        *it++ = (static_cast<uint8_t>(code >> 12) & 0x3f) | 0x80;
        *it++ = (static_cast<uint8_t>(code >> 6) & 0x3f) | 0x80;
        *it++ = (static_cast<uint8_t>(code) & 0x3f) | 0x80;
        break;
    }
    return it;
}

/* In accordance with the previous function */
constexpr unsigned short code_length(const utf8int_t code) {
    if (code < 0x80)
        return 1;
    else if (code < 0x800)
        return 2;
    else if (code < 0x10000)
        return 3;
    else if (code < 0x200000)
        return 4;
    else if (code < 0x4000000)
        return 5;
    else if (code < 0x80000000)
        return 6;
    return 0;
}





template<class Iter>
constexpr Iter _find_unicode_(Iter p, const size_t n, const unsigned cp)
{
    const auto e = p + n;
    while (p < e) {
        const auto len = sequence_length(p);
        if (len < 1 || len > 4 || p + len >= e)
            return nullptr;
        if (get_switch(p, len) == cp)
            return p;
        p += len;
    }
    return nullptr;
}

template<class Str>
constexpr size_t find_unicode(const Str &s_, const unsigned c_, const size_t pos_ = 0)
{
    const size_t size_ = s_.size();
    if (pos_ < size_)
    {
        const auto data_ = s_.data();
        const size_t n_ = size_ - pos_;
        const auto p_ = _find_unicode_(data_ + pos_, n_, c_);
        if (p_)
            return p_ - data_;
    }
    return Str::npos;
}

template<class Iter, class A, typename = std::enable_if_t<!std::is_arithmetic_v<A>>>
constexpr Iter _find_unicode_array_(Iter p, const size_t n, const A &cp)
{
    using value_type = std::decay_t<decltype(cp[0])>;
    static_assert(std::is_arithmetic_v<value_type> || std::is_enum_v<value_type>);
    const auto e = p + n;
    unsigned min_len = 4;
    for (value_type a : cp)
        min_len = std::min<unsigned>(min_len, utf8x::code_length(a));
    while (p < e) {
        const auto len = sequence_length(p);
        if (len < 1 || len > 4 || p + len >= e)
            return nullptr;
        if (len >= min_len) {
            const auto val = get_switch(p, len);
            for (value_type a : cp)
                if (val == a)
                    return p;
        }
        p += len;
    }
    return nullptr;
}

template<class Str, class N, typename = std::enable_if_t<!std::is_arithmetic_v<N>>>
constexpr size_t find_unicode_array(const Str &s_, N &&c_, const size_t pos_ = 0)
{
    const size_t size_ = s_.size();
    if (pos_ < size_)
    {
        const auto data_ = s_.data();
        const auto p_ = _find_unicode_array_(data_ + pos_, size_ - pos_, std::forward<N>(c_));
        if (p_)
            return p_ - data_;
    }
    return Str::npos;
}




template<typename A = char>
class translator
{
    using view_t = std::basic_string_view<A>;

    view_t _data;
    size_t _pos = 0;
    unsigned _len = 1;

    constexpr void get_length_(void) {
        _len = is_at_end() ? 0
            : sequence_length(&_data[_pos]);
        if (_len > 4 || _pos + _len > _data.size()) // Invalid sequence
            _len = 0;
    }

    constexpr void iterate_(void) { _pos += _len; get_length_(); }

public:

    struct end_sentinel_ {
        constexpr bool operator==(const translator &_t) {
            return _t.is_at_end();
        }
        constexpr bool operator!=(const translator &_t) {
            return !_t.is_at_end();
        }

        friend constexpr bool operator!=(end_sentinel_ _es, const translator &_t) {
            return !_t.is_at_end();
        }
        friend constexpr bool operator!=(const translator &_t, end_sentinel_ _es) {
            return !_t.is_at_end();
        }
    };

    constexpr translator() = default;
    constexpr translator(const translator &) = default;
    constexpr translator(translator &&) = default;
    constexpr translator &operator=(const translator &) = default;
    constexpr translator &operator=(translator &&) = default;

    constexpr translator(const view_t &_sv) : _data(remove_potential_BOM(_sv)), _pos(0), _len(1) { get_length_(); }
    constexpr translator &operator=(const view_t &_sv) { _data = _sv; _data = remove_potential_BOM(_data); _pos = 0; _len = 1; get_length_(); return *this; }

    constexpr bool is_at_end() const { return _len == 0 || _pos >= _data.size(); }
    constexpr utf8int_t get() const { return is_at_end() ? 0x0 : get_switch(&_data[_pos], _len); }
    constexpr utf8int_t get_and_iterate() { utf8int_t i = get(); iterate_(); return i; }
    constexpr operator utf8int_t() const { return get(); }
    constexpr auto get_pos() const { return _pos; }
    constexpr auto get_len() const { return _len; }
    constexpr void set_pos(size_t _p) { _pos = _p; get_length_(); }
    constexpr auto substr() { return _data.substr(_pos); }
    constexpr auto substr(size_t p, size_t s = view_t::npos) { return _data.substr(p, s); }
    constexpr auto size() const { return _data.size(); }

    constexpr auto operator*() const { return get(); }

    constexpr translator &operator++() {
        iterate_();
        return *this;
    }

    constexpr translator operator++(int) {
        translator copy{*this};
        this->iterate_();
        return copy;
    }

    constexpr end_sentinel_ end(void) { return {}; }

    constexpr translator begin(void) { return translator{_data}; }

    constexpr auto find_and_iterate(unsigned a) {
        _pos = std::min(find_unicode(_data, a, _pos), _data.size());
        get_length_();
        return _pos;
    }

    template<class N>
    constexpr auto find_and_iterate_array(N&&a) {
        _pos = std::min(find_unicode_array(_data, std::forward<N>(a), _pos), _data.size());
        get_length_();
        return _pos;
    }

    template<class Predicate>
    constexpr void skip_all(Predicate&&p) {
        while(!is_at_end() && p(get()))
            iterate_();
    }

    template<class Predicate>
    constexpr auto pop_substr_until(Predicate&&p) {
        const auto i = _pos;
        while(!(is_at_end() || p(get())))
            iterate_();
        return _data.substr(i, _pos - i);
    }

    constexpr void skip_whitespace(void) {
        while(!is_at_end() && _len == 1 && !!std::isspace(static_cast<unsigned char>(_data[_pos])))
            iterate_();
    }
};
}  // namespace utf8x
