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
#include <unordered_map>
#include <string_view>
#include <mutex>
#include <future>
#include <queue>

#include <math_defines.hpp>

namespace idle::images
{

struct loader
{
private:
    struct recipe_data
    {
        GLsizei width, height;
        GLint internalformat;
        GLenum format;
        GLint quality, repeat;
        std::unique_ptr<unsigned char[]> pixels;
        std::promise<GLuint> promise;
        GLenum error;

        template<typename Pixels, typename Promise>
        recipe_data(GLsizei w, GLsizei h,
                GLint i, GLenum f,
                GLint q, GLint r,
                Pixels&& pix,
                Promise&& prom
            ) noexcept :
            width(w), height(h),
            internalformat(i), format(f),
            quality(q), repeat(r),
            pixels(std::forward<Pixels>(pix)),
            promise(std::forward<Promise>(prom))
        {}
    };

    std::mutex queue_mutex;
    std::queue<recipe_data> load_queue;

public:
    GLuint load_from_memory(GLsizei w, GLsizei h,
                GLint i, GLenum f,
                GLint q, GLint r,
                std::unique_ptr<unsigned char[]> pix) noexcept;

    void load_topmost_queued_picture() noexcept;
};

struct database : loader
{
private:
    std::mutex map_mutex;
    std::unordered_map<std::string_view, GLuint> map;

public:
    GLuint load_from_assets(const char * fn, GLint quality) noexcept;

    void destroy_textures() noexcept;

    static void clean_trash() noexcept;
};

}  // namespace idle::images
