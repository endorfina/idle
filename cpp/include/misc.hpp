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
#include <random>
#include <optional>
#include <cassert>
#include "utf8.hpp"

namespace violet
{
    template<typename A>
    constexpr void skip_whitespaces(A *s, const size_t size, size_t &it) {
        for (; it < size && !!s[it] && !!std::isspace(static_cast<unsigned char>(s[it])); ++it);
    }

    template<class S>
    constexpr void skip_whitespaces(S&&s, size_t &it) {
        for (; it < s.size() && !!s[it] && !!std::isspace(static_cast<unsigned char>(s[it])); ++it);
    }

    template<class S>
    constexpr void skip_whitespaces(S &it, const S end) {
        while(it != end && *it <= 0x20)
            ++it;
    }

    constexpr int _cis_compare_(const std::string_view &lhs, const std::string_view &rhs) {
        const auto _s_ = std::min(lhs.size(), rhs.size());
        int _ret_ = 0;
        for (std::remove_const_t<decltype(_s_)> i = 0; i < _s_; ++i)
            if (const auto cl = ::tolower(lhs[i]), cr = ::tolower(rhs[i]);
                cl < cr) {
                _ret_ = -1;
                break;
            }
            else if (cl > cr) {
                _ret_ = 1;
                break;
            }
        if (_ret_ == 0) {
            if (lhs.size() < rhs.size())
            _ret_ = -1;
            else if (lhs.size() > rhs.size())
            _ret_ = 1;
        }
        return _ret_;
    }

    struct functor_less_comparator {
        bool operator()(const std::string &lhs, const std::string &rhs) const {
            return lhs < rhs;
        }
        bool operator()(const std::string_view &lhs, const std::string_view &rhs) const {
            return lhs < rhs;
        }
        using is_transparent = void;
    };
    struct functor_equal_comparator {
        bool operator()(const std::string &lhs, const std::string &rhs) const {
            return lhs == rhs;
        }
        bool operator()(const std::string_view &lhs, const std::string_view &rhs) const {
            return lhs == rhs;
        }
        using is_transparent = void;
    };
    struct cis_functor_equal_comparator {
        bool operator()(std::string_view lhs, std::string_view rhs) const {
            return _cis_compare_(lhs, rhs) == 0;
        }
        using is_transparent = void;
    };
    struct cis_functor_less_comparator {
        bool operator()(std::string_view lhs, std::string_view rhs) const {
            return _cis_compare_(lhs, rhs) < 0;
        }
        using is_transparent = void;
    };

    template<typename Char>
    constexpr bool _char_varbase_check_(const Char a, const unsigned int base) {
        return base > 10
            ? (a < 'A' ? a <= '9' && a >= '0' : a < (base - 10) + (a < 'a' ? 'A' : 'a'))
            : a >= '0' && a < ('0' + base);
    }

    template<typename Char>
    constexpr int _char_varbase_unsafe_(const Char a, const unsigned int base) {   // character has to be checked beforehand
        return base > 10 && a > Char('9')
            ? (a < Char('a') ? a - Char('A') : a - Char('a')) + 10
            : a - Char('0');
    }

    template<typename A, class View, typename = std::enable_if_t<std::is_arithmetic_v<A>>>
    constexpr ptrdiff_t svtonum(View&&v, A &out, unsigned int base = 0) noexcept
    {
        [[maybe_unused]] std::conditional_t<std::is_signed_v<A>, bool, const bool> is_negative = false;
        auto d = v.data();
        const auto e = d + v.size();

        assert(base != 1 && base < (10 + ('z' - 'a')));

        while (d != e && !!std::isspace(static_cast<unsigned char>(*d))) ++d;

        if constexpr (std::is_signed_v<A>) {
            if (d != e && *d == '-') {
                is_negative = true;
                while (++d != e && !!std::isspace(static_cast<unsigned char>(*d)));
            }
        }

        if (base == 0) {
            if (d == e || !std::isdigit(static_cast<unsigned char>(*d)))
                return v.size();
            if (*d == '0') {
                if (++d == e) {
                    out = 0;
                    return 0;
                }
                if (*d == 'x') {
                    base = 16;
                    ++d;
                }
                else if (!!std::isdigit(static_cast<unsigned char>(*d)))
                    base = 8;
                else
                    base = 10;
            }
            else base = 10;
        }
        else if (base == 16 && d + 1 != e && *d == '0' && *(d + 1) == 'x')
            d += 2;

        if (d == e || !_char_varbase_check_(*d, base))
            return v.size();
        out = _char_varbase_unsafe_(*d++, base);
        while (d != e && _char_varbase_check_(*d, base)) {
            out *= base;
            out += _char_varbase_unsafe_(*d++, base);
        }
        if constexpr (std::is_signed_v<A>) {
            if (is_negative)
                out *= -1;
        }
        return e - d;
    }

    template<typename A>
    constexpr const A * _find_(const A * p, const size_t n, const A c) {
        for (const auto e = p + n; p < e; ++p)
            if (*p == c)
                return p;
        return nullptr;
    }

    template<typename A>
    constexpr const A * _find7_(const A * p, const size_t n, const A c)
    {
        const auto e = p + n;
        while (p < e) {
            const auto len = utf8x::sequence_length(p);
            if (len > 1 && len <= 4) {
                p += len;
                continue;
            }
            else if (*p == c)
                return p;
            ++p;
        }
        return nullptr;
    }

    template<typename A>
    constexpr const A * _find7q_(const A * p, const size_t n, const A c)
    {
        const auto e = p + n;
        while (p < e) {
            const auto len = utf8x::sequence_length(p);
            const auto q = *p;
            if (len > 1 && len <= 4) {
                p += len;
                continue;
            }
            else if (q == c)
                return p;
            else if (q == '\"' || q == '\'')
                do { ++p; }
                while (*p != q && p < e);
            ++p;
        }
        return nullptr;
    }

    template<typename A>
    constexpr size_t find_skip_utf8(const std::basic_string_view<A>&_s_, const A _c_, const size_t _pos_ = 0)
    {
        size_t _ret_ = std::basic_string_view<A>::npos;
        const size_t _size_ = _s_.size();
        if (_pos_ < _size_)
        {
            const A * _data_ = _s_.data();
            const size_t _n_ = _size_ - _pos_;
            const A * _p_ = _find7_<A>(_data_ + _pos_, _n_, _c_);
            if (_p_)
            _ret_ = _p_ - _data_;
        }
        return _ret_;
    }

    template<typename A>
    constexpr size_t find_skip_utf8q(const std::basic_string_view<A>&_s_, const A _c_, const size_t _pos_ = 0)
    {
        size_t _ret_ = std::basic_string_view<A>::npos;
        const size_t _size_ = _s_.size();
        if (_pos_ < _size_)
        {
            const A * _data_ = _s_.data();
            const size_t _n_ = _size_ - _pos_;
            const A * _p_ = _find7q_<A>(_data_ + _pos_, _n_, _c_);
            if (_p_)
            _ret_ = _p_ - _data_;
        }
        return _ret_;
    }

    template<typename A>
    constexpr void remove_prefix_whitespace(std::basic_string_view<A> &_sv) {
        while (!!_sv.length() && !!std::isspace(static_cast<unsigned char>(_sv.front())))
                _sv.remove_prefix(1);
    }

    template<typename A>
    constexpr void remove_suffix_whitespace(std::basic_string_view<A> &_sv) {
        while (!!_sv.length() && !!std::isspace(static_cast<unsigned char>(_sv.back())))
                _sv.remove_suffix(1);
    }

    template<typename Int, typename Char>
    Char _random_char_(Int i) { // Int from 0 to 61 = 10 + 2 * 26 + 1
        return static_cast<Char>(i + (i < 10 ? '0' : (i < 36 ? 55 : 61)));
    }

    template<class Generator, typename CharIter>
    void generate_random_string(Generator&& gen, CharIter str, const size_t len) {
        std::uniform_int_distribution<short int> d(0, 10 + ('z' - 'a') + ('Z' - 'A'));
        for (const auto end = str + len; str != end; ++str)
            *str = _random_char_<short int, std::decay_t<decltype(*str)>>(d(gen));
    }

    template<typename CharIter>
    void generate_random_string(CharIter str, const size_t len) {
        std::random_device dev;
        std::minstd_rand gen{ dev() };
        generate_random_string(std::move(gen), str, len);
    }
}
