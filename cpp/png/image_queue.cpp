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

#include <atomic>
#include <log.hpp>

#include "image_queue.hpp"
#include "png.hpp"

namespace idle::images
{

GLuint database::load_from_assets(const char * filename, GLint quality) noexcept
{
    const std::string_view fn_view(filename);
    {
        std::lock_guard<std::mutex> lock(map_mutex);
        const auto iter = map.find(fn_view);
        if (iter != map.end())
        {
            return iter->second;
        }
    }

    {
        png_image_data picture(filename);

        if (!picture.image)
        {
            LOGE("Failed to load '%s'", filename);
            return 0;
        }

        LOGDD("Queuing %s texture creation", filename);

        const auto format = picture.size > 3 ? gl::RGBA : gl::RGB;
        const GLuint tex = load_from_memory(
                picture.real_width,
                picture.real_height,
                format,
                static_cast<GLenum>(format),
                quality,
                gl::CLAMP_TO_EDGE,
                std::move(picture.image));

        std::lock_guard<std::mutex> lock(map_mutex);
        map.try_emplace(fn_view, tex);
        return tex;
    }
}

GLuint loader::load_from_memory(GLsizei w, GLsizei h,
            GLint i, GLenum f,
            GLint q, GLint r,
            std::unique_ptr<unsigned char[]> pix) noexcept
{
    std::promise<GLuint> promise;
    auto tex_id = promise.get_future();
    {
        std::lock_guard<std::mutex> lock(queue_mutex);
        load_queue.emplace(
                w, h, i, f, q, r,
                std::move(pix), std::move(promise));
    }
    return tex_id.get();
}

void loader::load_topmost_queued_picture() noexcept
{
    if (auto td = [this]()->std::optional<recipe_data>
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            if (!!load_queue.size())
            {
                auto t = std::move(load_queue.front());
                load_queue.pop();
                LOGDD("Rendering topmost queued pixel data (%i x %i)", t.width, t.height);
                return { std::move(t) };
            }
            return {};
        }())
    {
        GLuint tex;
        gl::GenTextures(1, &tex);
        gl::BindTexture(gl::TEXTURE_2D, tex);

        gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_MIN_FILTER, td->quality);
        gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_MAG_FILTER, td->quality != gl::NEAREST ? gl::LINEAR : gl::NEAREST);
        gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_WRAP_S, td->repeat);
        gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_WRAP_T, td->repeat);
        gl::TexImage2D(gl::TEXTURE_2D, 0, td->internalformat, td->width, td->height, 0, td->format, gl::UNSIGNED_BYTE, td->pixels.get());

        gl::BindTexture(gl::TEXTURE_2D, 0);

        LOGDD("Tex id = %u", tex);
        td->promise.set_value(tex);
    }
}

database::~database() noexcept
{
    destroy_textures();
}

namespace
{

std::atomic<unsigned> trash_size = 0;
std::unique_ptr<GLuint[]> trash_content;

}  // namespace

void database::destroy_textures() noexcept
{
    std::lock_guard<std::mutex> lock(map_mutex);
    const auto size = map.size();
    auto out = std::make_unique<GLuint[]>(size);
    auto ptr = out.get();
    for (const auto& it : map)
    {
        LOGDD("Marking tex %u for destruction", it.second);
        *ptr++ = it.second;
    }
    map.clear();

    while (!!trash_size.load(std::memory_order_acquire)) std::this_thread::yield();

    trash_content = std::move(out);
    trash_size.store(static_cast<unsigned>(size), std::memory_order_release);
}

void database::clean_trash() noexcept
{
    if (const auto size = trash_size.load(std::memory_order_acquire))
    {
        LOGDD("Deleting %u textures", size);
        gl::DeleteTextures(size, trash_content.get());
        trash_content.reset(nullptr);
        trash_size.store(0, std::memory_order_release);
    }
}

}  // namespace idle::images

