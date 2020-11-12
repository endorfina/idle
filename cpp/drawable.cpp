/*
    Copyright © 2020 endorfina <dev.endorfina@outlook.com>

    This file is part of the Idle.

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

#include <cmath>
#include <optional>
#include <utility>
#include <future>
#include "gl.hpp"
#include "drawable.hpp"
#include "png.hpp"

namespace idle
{
namespace
{

constexpr float proportional_to_nearest_sqr(const size_t w) noexcept
{
    size_t u = 1;
    while (u < w) u *= 2;
    return static_cast<float>(w) / static_cast<float>(u);
}

struct image_meta_data_t
{
    GLsizei width, height;
    GLint internalformat;
    GLenum format;
    GLint quality, repeat;
    std::unique_ptr<unsigned char[]> pixels;
    std::promise<GLuint> promise;
    GLenum error;

    template<typename Pixels, typename Promise>
    image_meta_data_t(GLsizei w, GLsizei h,
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

std::vector<image_meta_data_t> load_queue;

std::mutex queue_mutex;

}  // namespace

image_t::image_t() noexcept
    : i(0), width(0), height(0)
{}

image_t::image_t(GLuint id, size_t w, size_t h) noexcept
    : tex(proportional_to_nearest_sqr(w), proportional_to_nearest_sqr(h)), i(id), width(w), height(h)
{}

image_t::image_t(GLuint id, size_t w, size_t h, size_t u2, size_t v2) noexcept
    : tex(static_cast<float>(w) / static_cast<float>(u2), static_cast<float>(h) / static_cast<float>(v2)), i(id), width(w), height(h)
{}

image_t::image_t(image_t&& other) noexcept :
    tex(other.tex), i(other.i), width(other.width), height(other.height)
{
    other.width = other.height = 0;
}

image_t::~image_t() noexcept
{
    if (width != 0 || height != 0)
        gl::DeleteTextures(1, &i);
}

GLuint image_t::get_gl_id() const noexcept
{
    return i;
}

GLuint image_t::release() noexcept
{
    width = height = 0;
    return i;
}


image_t image_t::load_from_assets_immediate(const char * fn, GLint quality) noexcept
{
    const png_image_data picture(fn);
    GLuint texID = 0;

    if (!picture.image)
    {
        LOGE("Failed to load '%s'", fn);
        return {};
    }

    gl::GenTextures(1, &texID);
    gl::BindTexture(gl::TEXTURE_2D, texID);
    gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_MIN_FILTER, quality); //gl::NEAREST = no smoothing
    gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_MAG_FILTER, quality != gl::NEAREST ? gl::LINEAR : gl::NEAREST); //gl::LINEAR = smoothing
    gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_WRAP_S, gl::CLAMP_TO_EDGE); // gl::CLAMP_TO_EDGE
    gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_WRAP_T, gl::CLAMP_TO_EDGE);
    gl::TexImage2D(gl::TEXTURE_2D, 0, picture.size > 3 ? gl::RGBA : gl::RGB, picture.real_width, picture.real_height, 0, picture.size > 3 ? gl::RGBA : gl::RGB, gl::UNSIGNED_BYTE, picture.image.get());

    if (graphics::assert_opengl_errors())
    {
        LOGE("Texture creation error: %s", fn);
        std::abort();
    }
    gl::BindTexture(gl::TEXTURE_2D, 0);

    return { texID, picture.width, picture.height, picture.real_width, picture.real_height };
}

image_t image_t::load_from_assets(const char * fn, GLint quality) noexcept
{
    png_image_data picture(fn);

    if (!picture.image)
    {
        LOGE("Failed to load '%s'", fn);
        return {};
    }

    std::promise<GLuint> promise;
    auto texID = promise.get_future();

    {
        const auto format = picture.size > 3 ? gl::RGBA : gl::RGB;
        std::lock_guard<std::mutex> lock(queue_mutex);
        load_queue.emplace_back(
            picture.real_width,
            picture.real_height,
            format,
            static_cast<GLenum>(format),
            quality,
            gl::CLAMP_TO_EDGE,
            std::move(picture.image),
            std::move(promise));
    }

    return { texID.get(), picture.width, picture.height, picture.real_width, picture.real_height };
}

image_t image_t::load_from_memory(GLsizei w, GLsizei h,
            GLint i, GLenum f,
            GLint q, GLint r,
            std::unique_ptr<unsigned char[]> pix) noexcept
{
    std::promise<GLuint> promise;
    auto texID = promise.get_future();

    {
        std::lock_guard<std::mutex> lock(queue_mutex);
        load_queue.emplace_back(
                w, h, i, f, q, r,
                std::move(pix), std::move(promise));
    }

    return { texID.get(), static_cast<size_t>(w), static_cast<size_t>(h), static_cast<size_t>(w), static_cast<size_t>(h) };
}

void image_t::load_topmost_queued_picture() noexcept
{
    if (auto td = []()->std::optional<image_meta_data_t>
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            if (!!load_queue.size())
            {
                auto t = std::move(load_queue.back());
                load_queue.pop_back();
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

        td->promise.set_value(tex);
    }
}



void fill_circle(const graphics::program_t& prog, point_t center, const float radius, const unsigned int steps) noexcept
{
    if (radius > 0.f && steps > 4)
    {
        point_t v[steps + 2];
        v[0] = {center.x, center.y};
        for (unsigned int i = 0; i <= steps; ++i) {
            const float a = i * math::tau / steps;
            v[1 + i] = {center.x + cosf(a) * radius, center.y + sinf(a) * radius};
        }

        prog.position_vertex(reinterpret_cast<const GLfloat*>(v));
        gl::DrawArrays(gl::TRIANGLE_FAN, 0, steps + 2);
    }
}

void draw_circle(const graphics::program_t& prog, point_t center, const float radius, const unsigned int steps) noexcept
{
    if (radius > 0.f && steps > 4)
    {
        point_t v[steps + 1];
        v[0] = {center.x, center.y};
        for (unsigned int i = 0; i <= steps; ++i) {
            const float a = i * math::tau / steps;
            v[i] = {center.x + cosf(a) * radius, center.y + sinf(a) * radius};
        }
        prog.position_vertex(reinterpret_cast<const GLfloat*>(v));
        gl::DrawArrays(gl::LINE_STRIP, 0, steps + 1);
    }
}

void fill_rectangle(const graphics::program_t& prog, const rect_t &rect) noexcept
{
    const float v[]
    {
            rect.left, rect.top,  rect.right, rect.top,
            rect.left, rect.bottom, rect.right, rect.bottom
    };

    prog.position_vertex(v);
    gl::DrawArrays(gl::TRIANGLE_STRIP, 0, 4);
}

void draw_rectangle(const graphics::program_t& prog, const rect_t &rect) noexcept
{
    const float v[]
    {
            rect.left, rect.top,  rect.right, rect.top,
            rect.right, rect.bottom, rect.left, rect.bottom,
            rect.left, rect.top
    };

    prog.position_vertex(v);
    gl::DrawArrays(gl::LINE_STRIP, 0, 5);
}

void fill_screen(const graphics::core& gl, const graphics::program_t& prog) noexcept
{
    prog.position_vertex(gl.draw_bounds_verts.data());
    gl::DrawArrays(gl::TRIANGLE_STRIP, 0, 4);
}

void fill_rectangle(const graphics::program_t& prog, point_t rect) noexcept
{
    const float v[]
    {
            0, 0,  rect.x, 0,
            0, rect.y, rect.x, rect.y
    };
    prog.position_vertex(v);
    gl::DrawArrays(gl::TRIANGLE_STRIP, 0, 4);
}

void draw_rectangle(const graphics::program_t& prog, point_t rect) noexcept
{
    const float v[]
    {
        0, 0,  rect.x, 0,
        rect.x, rect.y, 0, rect.y,
        0, 0
    };
    prog.position_vertex(v);
    gl::DrawArrays(gl::LINE_STRIP, 0, 5);
}

void fill_round_rectangle(const graphics::program_t& prog, const rect_t &rect, const float radius) noexcept
{
    if (radius <= 0.f)
    {
        fill_rectangle(prog, rect);
    }
    else
    {
        constexpr unsigned draw_steps = 5;
        std::array<point_t, (draw_steps + 1) * 4 + 2> v;
        v[0] = {(rect.left + rect.right) / 2, (rect.top + rect.bottom) / 2};

        for (unsigned i = 0; i <= draw_steps; ++i)
        {
            const float a = i * math::tau_4 / draw_steps;
            v[i + 1] = {rect.left + radius - cosf(a) * radius, rect.top + radius - sinf(a) * radius};
            v[i + 2 + draw_steps] = {rect.right - radius + sinf(a) * radius, rect.top + radius - cosf(a) * radius};
            v[i + 3 + draw_steps * 2] = {rect.right - radius + cosf(a) * radius, rect.bottom - radius + sinf(a) * radius};
            v[i + 4 + draw_steps * 3] = {rect.left + radius - sinf(a) * radius, rect.bottom - radius + cosf(a) * radius};
        }
        v[5 + draw_steps * 4] = {rect.left, rect.top + radius};

        prog.position_vertex(reinterpret_cast<const GLfloat*>(v.data()));
        gl::DrawArrays(gl::TRIANGLE_FAN, 0, v.size());
    }
}

void image_t::draw(const graphics::textured_program_t& prog) const noexcept
{
    const float v[]
    {
            0, 0,  static_cast<float>(width), 0,
            0, static_cast<float>(height), static_cast<float>(width), static_cast<float>(height)
    };
    const float t[]
    {
            0, 0,   tex.x, 0,
            0, tex.y, tex.x, tex.y
    };

    gl::ActiveTexture(gl::TEXTURE0);
    gl::BindTexture(gl::TEXTURE_2D, i);
    prog.position_vertex(v);
    prog.texture_vertex(t);
    gl::DrawArrays(gl::TRIANGLE_STRIP, 0, 4);
}

void image_t::draw(const graphics::textured_program_t& prog, point_t p) const noexcept
{
    const float v[]
    {
            p.x, p.y,  p.x + width, p.y,
            p.x, p.y + height, p.x + width, p.y + height
    };
    const float t[]
    {
            0, 0,   tex.x, 0,
            0, tex.y, tex.x, tex.y
    };
    gl::ActiveTexture(gl::TEXTURE0);
    gl::BindTexture(gl::TEXTURE_2D, i);
    prog.position_vertex(v);
    prog.texture_vertex(t);
    gl::DrawArrays(gl::TRIANGLE_STRIP, 0, 4);
}

void image_t::draw(const graphics::textured_program_t& prog, point_t p, const rect_t &rect) const noexcept
{
    const float w = fabsf(rect.right - rect.left), h = fabsf(rect.bottom - rect.top), tx = tex.x / width, ty = tex.y / height;
    const float v[]
    {
            p.x, p.y,  p.x + w, p.y,
            p.x, p.y + h, p.x + w, p.y + h
    };
    const float t[]
    {
            rect.left * tx, rect.top * ty,  rect.right * tx, rect.top * ty,
            rect.left * tx, rect.bottom * ty, rect.right * tx, rect.bottom * ty
    };
    gl::ActiveTexture(gl::TEXTURE0);
    gl::BindTexture(gl::TEXTURE_2D, i);
    prog.position_vertex(v);
    prog.texture_vertex(t);
    gl::DrawArrays(gl::TRIANGLE_STRIP, 0, 4);
}

void image_t::draw(const graphics::textured_program_t& prog, const rect_t &rect) const noexcept
{
    const float w = fabsf(rect.right - rect.left), h = fabsf(rect.bottom - rect.top), tx = tex.x / width, ty = tex.y / height;
    const float v[]
    {
            0, 0,  w, 0,
            0, h, w, h
    };
    const float t[]
    {
            rect.left * tx, rect.top * ty,  rect.right * tx, rect.top * ty,
            rect.left * tx, rect.bottom * ty, rect.right * tx, rect.bottom * ty
    };
    gl::ActiveTexture(gl::TEXTURE0);
    gl::BindTexture(gl::TEXTURE_2D, i);
    prog.position_vertex(v);
    prog.texture_vertex(t);
    gl::DrawArrays(gl::TRIANGLE_STRIP, 0, 4);
}

}  // namespace idle
