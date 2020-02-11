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
#include <memory>
#include <forward_list>
#include <unordered_map>
#include <map>

#include "top.hpp"
#include "drawable.hpp"
#include "platform/display.hpp"

#define MAX_VN_CHOICES 7
#define ANIMA_JOINTS 16
#define ANIMA_JOINT_DISPLACEMENTS 20

namespace idle {

    class object;

    class minimal_string {
        uint_least16_t _size = 0;           // 2 bytes
        std::unique_ptr<char[]> _data{};    // 8 bytes

        static uint_least16_t turncate(size_t s);

    public:
        using view_t = std::string_view;

        minimal_string(view_t str);

        minimal_string& operator=(view_t str);

        view_t view() const;
        operator view_t() const;
        auto size() const;

        struct compare {
            using is_transparent = int;
            bool operator()(const minimal_string& lhs, const minimal_string& rhs) const;
            bool operator()(const minimal_string& lhs, const view_t& rhs) const;
            bool operator()(const view_t& lhs, const minimal_string& rhs) const;
        };
    };
}

namespace std {
    template<> struct hash<idle::minimal_string> {
        using argument_type = idle::minimal_string;
        using result_type = size_t;

        inline auto operator()(argument_type const &str) const noexcept
        {
            return std::hash<std::string_view>{}(str.view());
        }
    };
}

namespace idle {
    struct line {
        enum class options_enum : unsigned char {
            Bg, // string_view
            Name, // string_view
            PortraitLeft, // string_view
            PortraitRight, // string_view
            Jump   // string_view
        };
        using options_pair_t = std::pair<options_enum, std::string_view>;
        using choices_pair_t = std::pair<std::string_view, minimal_string>;

        minimal_string str;
        std::forward_list<options_pair_t> options;
        std::forward_list<choices_pair_t> choices;

        line(minimal_string::view_t _s) : str{_s} {}
    };

    struct Database
    {
        using speech_key_map_t = std::unordered_map<minimal_string, unsigned int, std::hash<minimal_string>, minimal_string::compare>;
        speech_key_map_t speech_keys;

        //std::vector<sprite_t> sprites, portraits;
        std::map<std::string, unsigned> sprite_tags;
        //std::vector<image_t> tiles;
        //std::unordered_map<std::string_view, image_t> pictures;
        std::string file_font, file_models;


        struct _animation_ {
            struct _frame_ {
                float m[5][ANIMA_JOINTS];
            };
            std::vector<_frame_> f;
            bool repeat = false;
            float speed = .016f * 4 * APPLICATION_FPS;
        };
        typedef std::vector<_animation_> animation_cluster;
        struct joint_displacement {
            float m[ANIMA_JOINT_DISPLACEMENTS];
        };
        struct {
            std::vector<animation_cluster> swr;
            std::vector<joint_displacement> default_joint_displacement;

            bool _loaded = false;
        } animation_hub;

        static constexpr float accel = 40.f / APPLICATION_FPS;

        unsigned int get_sprite(const char *name) const;

        unsigned int get_vn_line(const char *key) const;

    private:
        bool vn_loaded = false;

        Database() = default;
        friend class ::overlay;

    public:
        bool load_everything(const ::platform::window& sys, ::overlay& parent);

        bool reload_animations();

        void reload_visuals(char * buff);
    };

    template<class Buffer>
    inline std::string_view read_data_with_var_header(Buffer&&b) {
        return b.read_data(b.read_utfx());
    }

    template<class Buffer>
    inline void write_data_with_var_header(Buffer&&b, const std::string_view &str) {
        b.write_utfx(str.size());
        b.write(str);
    }
}
