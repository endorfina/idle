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

#include <thread>
#include <unordered_set>
#include "platform/asset_access.hpp"
#include "top.hpp"
#include "database.hpp"
#include "blue/script.hpp"
#include "FreeType.hpp"

// #include <soloud.h>
// #include <soloud_modplug.h>

// void testsoloud() {
//     SoLoud::Soloud soloud;
//     SoLoud::Modplug mp;
//     soloud.init();
//     auto h = idle::parent.hold_asset("battlefield.xm");
//     const std::string_view data = *h;
//     if (auto err = mp.loadMem(data))
//         LOGE("Mod load: %s", soloud.getErrorString(err));}
//     soloud.play(mp);
//     std::this_thread::sleep_for(std::chrono::seconds(30))}
//     soloud.deinit();
// }


// #include "generated_textures.hpp"

namespace idle
{
namespace
{
// std::unordered_set<minimal_string, std::hash<minimal_string>, minimal_string::compare> unique_name_set;

// std::string_view unique(const std::string_view c)
// {
//     return unique_name_set.emplace(c).first->view();
// }
//
// std::string ultostr(const unsigned long u)
// {
//     char s[20];
//     snprintf(s, sizeof(s), "%lu", u);
//     return {s};
// }

// std::vector<line> load_speechlist(
//     const std::string_view _file,
//     Database::speech_key_map_t& speech_keys,
//     const std::unordered_map<std::string_view, std::string_view>& character_name_map)
// {
//     blue::parser p;
//     p << _file;
//     const auto& speechlist = std::get<blue::list>(p[u8"🎭"]);
//     std::vector<line> lines;
//     std::forward_list<line::options_pair_t> options;
//     LOGD("🎭 has %zu items", speechlist.size());
//     for (size_t i = 0; i < speechlist.size(); ++i)
//     {
//         if (const auto var = speechlist.yield<blue::dictionary>(i))
//         {
//             using op_t = line::options_enum;
//             if (const auto s = var->yield<blue::string_t>("name")) {
//                 auto fdcn = character_name_map.find(*s);
//                 options.emplace_front(op_t::Name, fdcn != character_name_map.end() ?
//                         fdcn->second : unique(*s));
//             }
//             if (const auto s = var->yield<blue::string_t>("bg")) {
//                 options.emplace_front(op_t::Bg, unique(*s));
//             }
//             if (const auto s = var->yield<blue::string_t>("left")) {
//                 options.emplace_front(op_t::PortraitLeft, unique(*s));
//             }
//             if (const auto s = var->yield<blue::string_t>("right")) {
//                 options.emplace_front(op_t::PortraitRight, unique(*s));
//             }
//         }
//         else if (const auto str = speechlist.yield<blue::string_t>(i))
//         {
//             if (str->size() < 2 || utf8x::sequence_length(str->data()) != str->size())
//             {
//                 lines.emplace_back(*str);
//                 if (!options.empty())
//                     lines.back().options = std::move(options);
//             }
//             else switch (utf8x::get_switch(str->data(), str->size())) {
//                 case blue::Codepoints::PushPin:
//                     if (const auto ptr = speechlist.yield<blue::string_t>(++i))
//                         speech_keys[*ptr] = lines.size();
//                     else throw std::runtime_error{"No text found at speech id " + ultostr(i)};
//                     break;
//                 case blue::Codepoints::Goat:
//                     if (const auto ptr = speechlist.yield<blue::string_t>(++i))
//                         lines.back().options.emplace_front(line::options_enum::Jump,
//                             std::string_view(speech_keys.try_emplace(*ptr).first->first));
//                     else throw std::runtime_error{"No text found at speech id " + ultostr(i)};
//                     break;
//                 case blue::Codepoints::Drama:
//                     if (speechlist.size() >= i + 2) {
//                         if (const auto key = speechlist.yield<blue::string_t>(i + 1),
//                                 label = speechlist.yield<blue::string_t>(i + 2);
//                                 key && label)
//                             lines.back().choices.emplace_front(
//                                 std::string_view(speech_keys.try_emplace(*key).first->first), *label);
//                     }
//                     i += 2;
//                     break;
//             }
//         }
//     }
//     lines.shrink_to_fit();
//     return lines;
// }

// std::unordered_map<std::string_view, std::string_view> create_character_name_map(blue::parser& parser, std::string_view var_name)
// {
//     std::unordered_map<std::string_view, std::string_view> out;
//     if (parser.has_defined(var_name))
//     {
//         auto& var = parser[var_name];
//         if (blue::yield_type(var) != blue::Types::Dictionary)
//             throw std::runtime_error{std::string(var_name) + " is not a dictionary"};
//
//         for (const auto& [name, type] : blue::cast<blue::dictionary&>(var)) {
//             const auto ptr = blue::get<blue::string_t>(&type);
//             if (!ptr)
//                 throw std::runtime_error{std::string(var_name) + "[ " + std::string(name) + " ] is not a string"};
//             out.try_emplace(name, unique_name_set.emplace(*ptr).first->view());
//         }
//     }
//     return out;
// }

std::optional<std::string> get_string_value(const blue::dictionary& dict, std::string_view name)
{
    if (auto f = dict.yield<blue::string_t>(name))
        return { static_cast<std::string>(*f) };
    return {};
}

}

bool Database::load_everything(const ::platform::window& sys, ::overlay& parent)
{
    const auto wtf = platform::asset::hold("wtf.txt");
    if (!wtf)
    {
        LOGE("Configuration file missing!");
        return false;
    }

    std::unordered_map<std::string_view, std::string_view> character_name_map;
    blue::parser p;
    const blue::dictionary* files = nullptr;
    p << wtf.view();

    if (!p.has_defined("files") || !(files = std::get_if<blue::dictionary>(&p["files"])))
    {
        LOGE("\"files\" dictionary must be defined.");
        return false;
    }

    const auto speechfile_fn = get_string_value(*files, "speech");
    if (!speechfile_fn)
    {
        LOGE("No speech file defined");
        return false;
    }

    const auto speechfile = platform::asset::hold(speechfile_fn->c_str());
    if (!speechfile)
    {
        LOGE("No speech file defined");
        return false;
    }

    if (auto mf = get_string_value(*files, "models"))
    {
        file_models = std::move(*mf);
    }
    else {
        LOGE("No model file defined");
    }

    if (auto ff = get_string_value(*files, "font"))
    {
        file_font = std::move(*ff);
        if (const auto fontfile = platform::asset::hold(file_font.c_str())) {
#ifndef __ANDROID__
            constexpr int resolution = 72;
#else
            constexpr int resolution = 48;
#endif
            if (auto ptr = fonts::FreeType{}.load(fontfile.view(), resolution))
            {
                parent.gl.font.emplace(std::move(*ptr));
            }
        }
        else
        {
            LOGE("Asset %s not found!", file_font.c_str());
            return false;
        }
    }
    else
    {
        LOGE("No font file defined");
        return false;
    }

    // load_speechlist(speechfile.view(), speech_keys, create_character_name_map(p, u8"🎤"));
    // parent.gl.image_id_noise = create_noise_texture();
    // parent.gl.image_id_fade = create_fade_texture();
    return true;
}


uint_least16_t minimal_string::turncate(size_t s) {
    return static_cast<uint_least16_t>(std::min<size_t>(s, UINT_LEAST16_MAX));
}

minimal_string::minimal_string(view_t str)
    : _size(turncate(str.size())),
    _data{std::make_unique<char[]>(_size)}
{
    memcpy(_data.get(), str.data(), _size);
}

minimal_string& minimal_string::operator=(view_t str)
{
    if (str.size() != _size) {
        _size = turncate(str.size());
        _data = std::make_unique<char[]>(_size);
    }
    memcpy(_data.get(), str.data(), _size);
    return *this;
}

minimal_string::view_t minimal_string::view() const { return { _data.get(), _size }; }
minimal_string::operator minimal_string::view_t() const { return { _data.get(), _size }; }
auto minimal_string::size() const { return _size; }

bool minimal_string::compare::operator()(const minimal_string& lhs, const minimal_string& rhs) const {
    return lhs.view() == rhs.view();
}
bool minimal_string::compare::operator()(const minimal_string& lhs, const minimal_string::view_t& rhs) const {
    return lhs.view() == rhs;
}
bool minimal_string::compare::operator()(const minimal_string::view_t& lhs, const minimal_string& rhs) const {
    return lhs == rhs.view();
}

}  // namespace idle
