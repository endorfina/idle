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

#include <charconv>
#include <string_view>
#include <vector>
#include <forward_list>

#include "types.hpp"
#include "builtin.hpp"
#include "what.hpp"
#include "script.hpp"

using namespace std::string_view_literals;

namespace blue
{
namespace
{
bool is_delimiter(const unsigned c) {
    return c == '\'' || c == '\"' || c == Codepoints::ABC;
}

bool _is_punct_lv(const unsigned c) {
    return c < 0x80 && c != '_' && c != '-' && c != '.' && (!!std::ispunct(static_cast<unsigned char>(c)));
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

std::string _format_where_(std::string s) {
    if (s.size() > 30) {
        return s.substr(0, std::min<size_t>(28, s.find('\n'))) + "...";
    }
    else if (!!s.size())
    {
        return s;
    }
    else
    {
        return "null";
    }
}

template<class T = string_t>
T _split_(std::string_view &_l) {
    const size_t clen = utf8x::sequence_length(_l.data());
    size_t len;
    T out;
    if (const auto _q_ = utf8x::get_switch(_l.data(), clen); is_delimiter(_q_)) {
        if (_l.size() < 2) {
            _l.remove_suffix(1);
        }
        else {
            len = utf8x::find_unicode(_l, _q_, clen);
            if (len > clen)
                out = _l.substr(clen, len - clen);
            _l.remove_prefix(std::min(len + clen, _l.length()));
            remove_prefix_whitespace(_l);
        }
    }
    else {
        len = 1;
        if (clen > 1 || !_is_punct_lv(_l.front())) {
            if (clen == 1) {
                if ((_q_ >= '0' && _q_ <= '9') || _q_ == '-' || _q_ == '.')
                {
                    bool found_decimal = _q_ == '.';
                    while (len < _l.length() &&
                            utf8x::sequence_length(_l.data() + len) == 1 &&
                            (_l[len] == '.' || (_l[len] >= '0' && _l[len] <= '9'))) {
                        if (_l[len] == '.')
                        {
                            if (found_decimal)
                                break;
                            found_decimal = true;
                        }
                        ++len;
                    }
                }
                else while (len < _l.length() &&
                        utf8x::sequence_length(_l.data() + len) == 1 &&
                        !_is_punct_lv(_l[len]) &&
                        !std::isspace(static_cast<unsigned char>(_l[len])))
                    ++len;
            }
            else if (clen <= _l.length()) {
                if constexpr (!std::is_same_v<T, string_t>) {
                    out = _q_;
                    _l.remove_prefix(clen);
                    remove_prefix_whitespace(_l);
                    return out;
                }
                else len = clen;
            }
        }
        else if (_l.length() > 1 && (_l[0] == '<' || _l[0] == '>' || _l[0] == '=') && _l[1] == '=')
            ++len;

        out = _l.substr(0, len);

        _l.remove_prefix(len);

        remove_prefix_whitespace(_l);
    }
    return out;
}

void skip_trash(utf8x::translator<char>& uc) {
    uc.skip_whitespace();
    while (*uc == 0xfe0f) {
        (++uc).skip_whitespace();
    }
}

template<class T = string_t>
T pop(utf8x::translator<char>& _uc) {
    std::string_view t = _uc.substr();
    T rtn = _split_<T>(t);
    _uc = t;
    return rtn;
}

// struct default_includer
// {
//     std::forward_list<std::vector<char>> files;

//     std::string_view operator()(const std::string& filename)
//     {
//         std::vector<char> out;
//         violet::internal::read_from_file(filename.c_str(), out);
//         if (!!out.size())
//         {
//             std::string_view view{ out.data(), out.size() };
//             files.push_front(std::move(out));
//             return view;
//         }
//         return {};
//     }
// };

}  // namespace

// parser::parser() : include(default_includer{})
// {}

parser& parser::operator<<(utf8x::translator<char> src) {
    while (!src.is_at_end())
        parse(src, true);
    return *this;
}

const variable_t& parser::operator[](std::string_view key) const {
    return names.at(key);
}

variable_t& parser::operator[](std::string_view key) {
    return names.at(key);
}

void parser::report_failure(const char * title, std::string_view code)
{
    failed = true;
    error_string = title + (", at:\n" + _format_where_(static_cast<std::string>(code)));
}

return_t parser::parse(utf8x::translator<char>& uc, bool evaluate)
{
    if (failed)
        return {};

    skip_trash(uc);
    if (uc.is_at_end())
        return {};
    const auto u8c = *uc;
    const auto _er = uc.substr();

    switch (u8c) {
    case Codepoints::Ghost:
        if (*++uc == Codepoints::Grape) {
            do {
                uc.find_and_iterate(Codepoints::Watermelon);
            } while (!uc.is_at_end() && ++uc != Codepoints::Ghost);
        }
        else uc.find_and_iterate('\n');
        if (!(++uc).is_at_end())
            return parse(uc, evaluate);
        return {};

    // case '#':
    //     uc.find_and_iterate('\n');
    //     if (!(++uc).is_at_end())
    //         return parse(uc, evaluate);
    //     return {};

    case Codepoints::Thinking:
        skip_trash(++uc);
        switch (uc.get_and_iterate()) {
            case Codepoints::Ok:
                return { Types::Bool };
            case Codepoints::Locomotive:
                return { Types::Integer };
            case Codepoints::Rocket:
                return { Types::Float };
            case Codepoints::ABC:
                return { Types::String };
            // case Codepoints::Candy:
            //     return { Types::Function };
            case Codepoints::Honey:
                return { Types::Dictionary };
            case Codepoints::IceCream:
                return { Types::List };
            default:
                report_failure("Wrong type", _er);
                return {};
        }
        break;

    // case Codepoints::SpaceInvader: // extern
    //     ++uc;
    //     if (!evaluate)
    //         break;
    //     // if (!get_extern)
    //     //     report_failure("Internal error. Callback hasn't been defined", _er};
    //     if (auto name = parse(uc))
    //         return { get_extern(cast<std::string_view>(*name)) };
    //     else
    //         report_failure("External requires a string argument", _er};
    //     break;

    case Codepoints::SoftIceCream:
        ++uc;
        if (!evaluate)
            break;
        skip_trash(uc);
        if (const auto _c = *uc; is_delimiter(_c))
        {
            report_failure("Quotes not allowed in names", _er);
            return {};
        }
        else if (_c == '*')
        {
            report_failure("Name cannot start with an asterisk", _er);
            return {};
        }
        else {
            const std::string_view name = pop(uc);
            if (auto f = names.find(name); f != names.end())
            {
                report_failure("Double declaration", _er);
                return {};
            }
            if (auto rtn = parse(uc))
                names.insert_or_assign(name, std::move(*rtn));
            else
            {
                report_failure("Cannot assign nothing", _er);
                return {};
            }
        }
        break;

    case Codepoints::IceCream: {
            list l;
            ++uc;
            while(true) {
                skip_trash(uc);
                if (*uc == Codepoints::Eggplant) {
                    ++uc;
                    if (!evaluate)
                        return {};
                    break;
                }
                auto rtn = parse(uc, evaluate);
                if (evaluate) {
                    if (rtn)
                        l.push_back(std::move(*rtn));
                    else
                    {
                        report_failure("Cannot assign nothing", _er);
                        return {};
                    }
                }
            }
            return { std::move(l) };
        }
        break;

    case Codepoints::Angry:
        ++uc;
        if (!evaluate)
            break;
        if (auto val = parse(uc))
            return { !cast<bool_t>(*val) };
        else
        {
            report_failure("Bool expected", _er);
            return {};
        }
        break;

    case Codepoints::Honey: {
            dictionary m;
            ++uc;
            while(true) {
                skip_trash(uc);
                if (*uc == Codepoints::Eggplant) {
                    ++uc;
                    if (!evaluate)
                        return {};
                    break;
                }
                string_t *name = nullptr;
                auto rtn = parse(uc, evaluate);
                if (evaluate) {
                    if (rtn && (name = std::get_if<string_t>(&*rtn))) {
                        skip_trash(uc);
                        if (*uc == Codepoints::Eggplant)
                        {
                            report_failure("An even amount of arguments needed for a key-value pair", _er);
                            return {};
                        }
                        else if (auto rtn2 = parse(uc))
                            m.push(*name, std::move(*rtn2));
                        else
                        {
                            report_failure("Cannot assign nothing", _er);
                            return {};
                        }
                    }
                    else
                    {
                        report_failure("Key in key-value pair must be a string", _er);
                        return {};
                    }
                }
            }
            return { std::move(m) };
        }
        break;

    // case Codepoints::Bell:
    //     ++uc;
    //     if (!evaluate)
    //         break;
    //     skip_trash(uc);
    //     {
    //         evoker ev;
    //         while (!uc.is_at_end() && *uc != Codepoints::Eggplant) {
    //             if (auto k1 = parse(uc))
    //                 if (auto t1 = std::get_if<Types>(&*k1)) {
    //                     skip_trash(uc);
    //                     if (!uc.is_at_end() && *uc != Codepoints::Eggplant)
    //                         if (auto k2 = parse(uc))
    //                             if (auto t2 = std::get_if<function>(&*k2)) {
    //                                 // TODO: check if type already exists
    //                                 ev.evoked.insert_or_assign(*t1, *t2);
    //                                 if (*t1 == Types::Function)
    //                                     ev.has_function_handler = true;
    //                                 continue;
    //                             }
    //                 }
    //             report_failure( "Evoker needs a type and a function", _er};
    //         }
    //         ++uc;
    //         return function{ std::move(ev) };
    //     }
    //     break;

    // case Codepoints::Lollipop:
    //     ++uc;
    //     if (!evaluate)
    //         break;
    //     if (auto f = parse(uc); f && yield_type(*f) == Types::Function) {
    //         auto& fun = cast<const function&>(*f);
    //         args_t a{ fun.arg_count > 0 ? new variable_t[fun.arg_count] : nullptr };
    //         for (unsigned int n = 0; n < fun.arg_count; ++n) {
    //             if (auto o = parse(uc))
    //                 a[n] = std::move(*o);
    //             else
    //                 report_failure("One or more arguments unevalueable", _er};
    //         }
    //         return fun.call(std::move(a));
    //     }
    //     report_failure( "Wrong callable", _er};
    //     break;

    // case Codepoints::Scroll:
    //     ++uc;
    //     if (!evaluate)
    //         break;
    //     if (!include)
    //         report_failure("Internal error. Callback hasn't been defined", _er};
    //     if (auto f = parse(uc))
    //         if (auto fn = cast<std::string>(*f); !!fn.size())
    //             if (const auto s = include(fn); !!s.size())
    //             {
    //                 utf8x::translator<char> cuc{s};
    //                 while(!cuc.is_at_end())
    //                     parse(cuc);
    //                 return {};
    //             }
    //     report_failure( "Could not load file", _er};
    //     break;

    case Codepoints::Tangerine:
        ++uc;
        if (!evaluate)
            break;
        if (auto k = parse(uc)) {
            auto cond = cast<bool_t>(*k);
            parse(uc, cond);
            skip_trash(uc);
            while (!cond && *uc == Codepoints::Lemon) {
                k = parse(++uc, true);
                cond = cast<bool_t>(*k);
                parse(uc, cond);
                skip_trash(uc);
            }
            while (*uc == Codepoints::Lemon) {
                parse(++uc, false);
                parse(uc, false);
                skip_trash(uc);
            }
            if (*uc == Codepoints::Strawberry)
                parse(++uc, !cond);
            break;
        }
        report_failure( "No condition provided", _er);
        return {};

    case Codepoints::Strawberry:
    case Codepoints::Lemon:
        if (evaluate)
        {
            report_failure( "Out of place else statement", _er);
            return {};
        }
        ++uc;
        break;

    case Codepoints::Watermelon:
        report_failure( "Out of place watermelon", _er);
        return {};

    case Codepoints::Grape:
        skip_trash(++uc);
        while (!uc.is_at_end() && *uc != Codepoints::Watermelon) {
            parse(uc, evaluate);
            skip_trash(uc);
        }
        ++uc;
        break;

    case Codepoints::ThumbsUp:
        ++uc;
        return { true };

    case Codepoints::ThumbsDown:
        ++uc;
        return { false };

    default:
        {
            const std::string_view str = pop(uc);
            if (evaluate && !is_delimiter(u8c) && !(u8c >= '0' && u8c <= '9'))
                if (auto k = names.find(str); k != names.end()) {
                    if (auto ref = std::get_if<reference>(&k->second))
                        return { k->second };
                    return { reference{ &k->second, nullptr } };
                }

            if (get_lines || u8c == '*') {
                uc = _er;
                if (u8c == '*')
                 ++uc;
                return { uc.pop_substr_until([](auto c){ return c == '\n'; }) };
            }
            if ((u8c >= '0' && u8c <= '9') || (u8c == '-' && str.size() > 1) || u8c == '.')
            {
                if (auto decimal = str.find('.'); decimal < str.size())
                {
                    std::string s{str};
                    if (decimal == 0 || str[decimal - 1] < '0' || str[decimal - 1] > '9')
                        s.insert(decimal, 1, '0');
                    return { float_t(std::stof(s)) };
                }
                return { int_t(std::stol(std::string(str))) };
            }
            return { str };
        }
    }
    return {};
}

bool parser::has_defined(std::string_view key) const {
    const auto f = names.find(key);
    return f != names.end();
}

namespace {

// std::string& operator<<(std::string& out, Codepoints code) {
//     const auto size = out.size();
//     const unsigned int c = utf8x::code_length(code);
//     out.resize(size + c);
//     utf8x::put_switch(out.data() + size, c, code);
//     return out;
// }

template<utf8int_t Val>
constexpr auto _uarr_f(void)
{
    std::array<char, utf8x::code_length(Val)> a{};
    utf8x::put(a.data(), Val);
    return a;
}

template<utf8int_t Val>
static constexpr auto _uarr_ = _uarr_f<Val>();

template<utf8int_t Val>
constexpr std::string_view uarr()
{
    return { _uarr_<Val>.data(), _uarr_<Val>.size() };
}

void print_var(const Types* ptr, void (*callback)(void*, std::string_view), void* const param);

void print_(const dictionary& dict, void (*callback)(void*, std::string_view), void* const param)
{
    callback(param, uarr<Codepoints::Honey>());
    for (const auto& [name, type] : dict) {
        callback(param, uarr<Codepoints::ABC>());
        callback(param, name);
        callback(param, uarr<Codepoints::ABC>());
        print_var(&type, callback, param);
    }
    callback(param, uarr<Codepoints::Eggplant>());
}

void print_(const list& l, void (*callback)(void*, std::string_view), void* const param)
{
    callback(param, uarr<Codepoints::IceCream>());
    if (!!l.size()) {
        print_var(l[0], callback, param);
    }
    for (unsigned i = 1; i < l.size(); ++i) {
        callback(param, " ");
        print_var(l[i], callback, param);
    }
    callback(param, uarr<Codepoints::Eggplant>());
}

void print_(const bool_t arg, void (*callback)(void*, std::string_view), void* const param)
{
    callback(param, arg ? uarr<Codepoints::ThumbsUp>() : uarr<Codepoints::ThumbsDown>());
}
void print_(const int_t arg, void (*callback)(void*, std::string_view), void* const param)
{
    char str[24];
    callback(param, {str, static_cast<size_t>(snprintf(str, sizeof(str), "%ld", arg))});
}

void print_(const float_t arg, void (*callback)(void*, std::string_view), void* const param)
{
    char str[32];
    for (int n = snprintf(str, 32, "%f", arg) - 1; n > 0 && str[n] == '0'; --n)
        str[n] = 0x0;
    const char * ptr = str;
    if (str[0] == '-' && str[1] == '0')
    {
        str[1] = '-';
        ++ptr;
    }
    else if (str[0] == '0' && str[1] == '.')
        ++ptr;
    callback(param, {ptr});
}

void print_(const string_t arg, void (*callback)(void*, std::string_view), void* const param)
{
    callback(param, uarr<Codepoints::ABC>());
    callback(param, arg);
    callback(param, uarr<Codepoints::ABC>());
}

void _to_str_callback_(void* param, std::string_view add)
{
    std::string& str = *reinterpret_cast<std::string*>(param);
    str.append(add.data(), add.size());
}

void print_var(const Types* ptr, void (*callback)(void*, std::string_view), void* const param)
{
    // long, float, std::string_view, bool, Types, function, dictionary, list, object, reference
    switch(*ptr)
    {
        case Types::Integer:
            print_(*reinterpret_cast<const int_t*>(++ptr), callback, param);
            break;
        case Types::Float:
            print_(*reinterpret_cast<const float_t*>(++ptr), callback, param);
            break;
        case Types::String:
            print_(*reinterpret_cast<const string_t*>(++ptr), callback, param);
            break;
        case Types::Bool:
            print_(*reinterpret_cast<const bool_t*>(++ptr), callback, param);
            break;
        case Types::Dictionary:
            print_(*reinterpret_cast<const dictionary*>(++ptr), callback, param);
            break;
        case Types::List:
            print_(*reinterpret_cast<const list*>(++ptr), callback, param);
            break;
        default:
            std::abort();
    }
}

}  // namespace

void print(const variable_t &var, void (*callback)(void*, std::string_view), void* const param)
{
    std::visit([callback, param](const auto& arg)->void {
        using T = std::remove_cv_t<std::remove_reference_t<decltype(arg)>>;
        if constexpr (std::is_same_v<T, reference>) {
            if (arg.variant_data)
                print(*reinterpret_cast<variable_t*>(arg.variant_data), callback, param);
            else if (arg.container_data)
                print_var(arg.container_data, callback, param);
            std::abort();
        }
        else if constexpr (std::is_same_v<T, Types>) {
            std::abort();
        }
        else {
            print_(arg, callback, param);
        }
    }, var);
}

std::string print_to_str(const variable_t &var)
{
    std::string out;
    print(var, _to_str_callback_, &out);
    return out;
}

// function callback_extern::operator()(std::string_view name) {
//     if (auto find = _db.find(name); find != _db.end()) {
//         auto &[count, ptr] = find->second;
//         function f;
//         f.arg_count = count;
//         f.ptr = ptr.get();
//         f.is_evoker = false;
//         return f;
//     }
//     return {};
// }

}  // namespace blue
