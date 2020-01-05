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

#include <optional>
#include <utility>
#include <memory>
#include <future>
#include <vector>
#include <zlib.hpp>
#include "gl.hpp"
#include "platform/asset_access.hpp"
#include "platform/display.hpp"
#include "drawable.hpp"
#include "lodepng.h"

namespace idle
{
namespace
{

float proportional_to_nearest_sqr(const size_t w)
{
    size_t u = 1;
    while (u < w) u *= 2;
    return static_cast<float>(w) / static_cast<float>(u);
}

unsigned wasteful_zlib_decompress(unsigned char** out_ptr, size_t* out_size,
                                 const unsigned char* const source,
                                 const size_t source_size,
                                 const LodePNGDecompressSettings* const)
{
    if (const auto working_buffer = zlib<std::vector<unsigned char>>(source, source_size, false, false))
    {
        std::unique_ptr<unsigned char[]> temporary_buffer = std::make_unique<unsigned char[]>(working_buffer->size());
        ::memcpy(temporary_buffer.get(), working_buffer->data(), working_buffer->size());

        // I hope I understand this correctly, lmao
        *out_ptr = temporary_buffer.release();
        *out_size = working_buffer->size();
        return 0;
    }
    return 1;
}

struct image_data
{
    std::unique_ptr<unsigned char[]> image;
    unsigned width, height, real_width, real_height, size;

    std::vector<unsigned char> png(const unsigned char * const src, const size_t datalen)
    {
        std::vector<unsigned char> out;
        lodepng::State st;
        st.info_raw.colortype = LodePNGColorType::LCT_RGBA;
        st.decoder.zlibsettings.custom_zlib = wasteful_zlib_decompress;

        if (lodepng::decode(out, width, height, st, src, datalen) != 0)
        {
            out.clear();
        }
        size = st.info_png.color.colortype == LodePNGColorType::LCT_RGBA ? 4 : 3;
        return out;
    }

    image_data() = default;

    image_data(const std::string_view source)
    {
        constexpr unsigned source_size = 4;
        if (const auto temp = png(reinterpret_cast<const unsigned char*>(source.data()), source.size()); !!temp.size())
        {
            real_width = 1, real_height = 1;
            while (real_width < width) real_width *= 2;
            while (real_height < height) real_height *= 2;

            image = std::make_unique<unsigned char[]>(real_width * real_height * size);
            ::memset(image.get(), 0, real_width * real_height * size);

            if (source_size != size)
            {
                for (unsigned y = 0; y < height; ++y)
                for (unsigned x = 0; x < width; ++x)
                    ::memcpy(image.get() + size * (real_width * y + x),
                        temp.data() + source_size * (width * y + x),
                        size);
            }
            else
            {
                for (unsigned y = 0; y < height; ++y)
                    ::memcpy(image.get() + size * real_width * y,
                        temp.data() + source_size * width * y,
                        source_size * width);
            }
        }
        else
        {
            LOGE("PNG fail");
        }
    }
};

bool verify_file_extension(const char * const fn)
{
    auto chk = fn;
    for (; *chk; ++chk);
    for (; chk != fn && *chk != '.'; --chk);
    return !(strcmp(chk, ".png") != 0);
}

image_data make_pic(const char * fn)
{
    if (!verify_file_extension(fn))
    {
        return {};
    }

    if (const auto b = platform::asset::hold(fn))
    {
        return { b.view() };
    }

    return {};
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
        ) :
        width(w), height(h),
        internalformat(i), format(f),
        quality(q), repeat(r),
        pixels(std::forward<Pixels>(pix)),
        promise(std::forward<Promise>(prom))
    {}

    image_meta_data_t(const image_meta_data_t&) = delete;
    image_meta_data_t(image_meta_data_t&&) = default;
};

std::vector<image_meta_data_t> load_queue;

std::mutex queue_mutex;

}  // namespace

image_t::image_t()
    : i(0), width(0), height(0)
{}

image_t::image_t(GLuint id, size_t w, size_t h)
    : tex(proportional_to_nearest_sqr(w), proportional_to_nearest_sqr(h)), i(id), width(w), height(h)
{}

image_t::image_t(GLuint id, size_t w, size_t h, size_t u2, size_t v2)
    : tex(static_cast<float>(w) / static_cast<float>(u2), static_cast<float>(h) / static_cast<float>(v2)), i(id), width(w), height(h)
{}

image_t::image_t(image_t&& other) :
    tex(other.tex), i(other.i), width(other.width), height(other.height)
{
    other.width = other.height = 0;
}

image_t::~image_t()
{
    if (width != 0 || height != 0)
        gl::DeleteTextures(1, &i);
}

GLuint image_t::get_gl_id() const
{
    return i;
}

GLuint image_t::release()
{
    width = height = 0;
    return i;
}


image_t image_t::load_from_assets_immediate(const char * fn, GLint quality)
{
    const auto picture = make_pic(fn);
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

    if (assertGlErrors())
    {
        LOGE("Texture creation error: %s", fn);
        std::abort();
    }
    gl::BindTexture(gl::TEXTURE_2D, 0);

    return { texID, picture.width, picture.height, picture.real_width, picture.real_height };
}

image_t image_t::load_from_assets(const char * fn, GLint quality)
{
    auto picture = make_pic(fn);

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
            std::unique_ptr<unsigned char[]> pix)
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

void image_t::load_topmost_queued_picture()
{
    if (auto td = []()->std::optional<image_meta_data_t>
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            if (!!load_queue.size())
            {
                auto t = std::move(load_queue.back());
                load_queue.pop_back();
                LOGD("Rendering topmost queued pixel data (%i x %i)", t.width, t.height);
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



void sprite_t::add_subsprite(std::vector<sprite_t> &out, const image_t& image, unsigned x, unsigned y, unsigned w, unsigned h, point_t c, point_t o) {
    const point_t size(static_cast<float>(w), static_cast<float>(h));
    const float m_x = image.tex.x / static_cast<float>(image.width),
            m_y = image.tex.y / static_cast<float>(image.height);
    for (unsigned iy = 0, ix; iy < y; ++iy)
        for (ix = 0; ix < x; ++ix)
            out.emplace_back(image.i, rect_t((o.x + ix * w) * m_x,
                                   (o.y + iy * h) * m_y,
                                   (o.x + (ix + 1) * w) * m_x,
                                   (o.y + (iy + 1) * h) * m_y),
                             c, size, point_t(m_x, m_y));
}

void fill_circle(const graphics::program_t& prog, point_t center, const float radius, const unsigned int steps)
{
    if (radius > 0.f && steps > 4)
    {
// #ifdef WIN32
//             std::vector<float> _v((steps + 2) * 2);
//             float * const v = _v.data();
// #else
        point_t v[steps + 2];
        v[0] = {center.x, center.y};
        for (unsigned int i = 0; i <= steps; ++i) {
            const float a = i * F_TAU / steps;
            v[1 + i] = {center.x + cosf(a) * radius, center.y + sinf(a) * radius};
        }

        prog.position_vertex(reinterpret_cast<const GLfloat*>(v));
        gl::DrawArrays(gl::TRIANGLE_FAN, 0, steps + 2);
    }
}

void draw_circle(const graphics::program_t& prog, point_t center, const float radius, const unsigned int steps)
{
    if (radius > 0.f && steps > 4)
    {
        point_t v[steps + 1];
        v[0] = {center.x, center.y};
        for (unsigned int i = 0; i <= steps; ++i) {
            const float a = i * F_TAU / steps;
            v[i] = {center.x + cosf(a) * radius, center.y + sinf(a) * radius};
        }
        prog.position_vertex(reinterpret_cast<const GLfloat*>(v));
        gl::DrawArrays(gl::LINE_STRIP, 0, steps + 1);
    }
}

void fill_rectangle(const graphics::program_t& prog, const rect_t &rect)
{
    const float v[] = {
            rect.left, rect.top,  rect.right, rect.top,
            rect.left, rect.bottom, rect.right, rect.bottom
    };

    prog.position_vertex(v);
    gl::DrawArrays(gl::TRIANGLE_STRIP, 0, 4);
}

void draw_rectangle(const graphics::program_t& prog, const rect_t &rect)
{
    const float v[] = {
            rect.left, rect.top,  rect.right, rect.top,
            rect.right, rect.bottom, rect.left, rect.bottom,
            rect.left, rect.top
    };

    prog.position_vertex(v);
    gl::DrawArrays(gl::LINE_STRIP, 0, 5);
}

void fill_rectangle(const graphics::program_t& prog, point_t rect)
{
    const float v[] = {
            0, 0,  rect.x, 0,
            0, rect.y, rect.x, rect.y
    };
    prog.position_vertex(v);
    gl::DrawArrays(gl::TRIANGLE_STRIP, 0, 4);
}

void draw_rectangle(const graphics::program_t& prog, point_t rect)
{
    const float v[] = {
            0, 0,  rect.x, 0,
            rect.x, rect.y, 0, rect.y,
            0, 0
    };
    prog.position_vertex(v);
    gl::DrawArrays(gl::LINE_STRIP, 0, 5);
}

#define ROUND_RECTANGLE_DRAW_STEPS 5

void fill_round_rectangle(const graphics::program_t& prog, const rect_t &rect, const float radius)
{
    if (radius <= 0.f) {
        fill_rectangle(prog, rect);
    }
    else {
        std::array<point_t, (ROUND_RECTANGLE_DRAW_STEPS + 1) * 4 + 2> v;
        v[0] = {(rect.left + rect.right) / 2, (rect.top + rect.bottom) / 2};
        for (unsigned i = 0; i <= ROUND_RECTANGLE_DRAW_STEPS; ++i) {
            const float a = i * F_TAU_4 / ROUND_RECTANGLE_DRAW_STEPS;
            v[i + 1] = {rect.left + radius - cosf(a) * radius, rect.top + radius - sinf(a) * radius};
            v[i + 2 + ROUND_RECTANGLE_DRAW_STEPS] = {rect.right - radius + sinf(a) * radius, rect.top + radius - cosf(a) * radius};
            v[i + 3 + ROUND_RECTANGLE_DRAW_STEPS * 2] = {rect.right - radius + cosf(a) * radius, rect.bottom - radius + sinf(a) * radius};
            v[i + 4 + ROUND_RECTANGLE_DRAW_STEPS * 3] = {rect.left + radius - sinf(a) * radius, rect.bottom - radius + cosf(a) * radius};
        }
        v[5 + ROUND_RECTANGLE_DRAW_STEPS * 4] = {rect.left, rect.top + radius};

        prog.position_vertex(reinterpret_cast<const GLfloat*>(v.data()));
        gl::DrawArrays(gl::TRIANGLE_FAN, 0, v.size());
    }
}

void image_t::draw(const graphics::textured_program_t& prog) const
{
    const float v[] = {
            0, 0,  static_cast<float>(width), 0,
            0, static_cast<float>(height), static_cast<float>(width), static_cast<float>(height)
    };
    const float t[] = {
            0, 0,   tex.x, 0,
            0, tex.y, tex.x, tex.y
    };

    gl::ActiveTexture(gl::TEXTURE0);
    gl::BindTexture(gl::TEXTURE_2D, i);
    prog.position_vertex(v);
    prog.texture_vertex(t);
    gl::DrawArrays(gl::TRIANGLE_STRIP, 0, 4);
}

void image_t::draw(const graphics::textured_program_t& prog, point_t p) const
{
    const float v[] = {
            p.x, p.y,  p.x + width, p.y,
            p.x, p.y + height, p.x + width, p.y + height
    };
    const float t[] = {
            0, 0,   tex.x, 0,
            0, tex.y, tex.x, tex.y
    };
    gl::ActiveTexture(gl::TEXTURE0);
    gl::BindTexture(gl::TEXTURE_2D, i);
    prog.position_vertex(v);
    prog.texture_vertex(t);
    gl::DrawArrays(gl::TRIANGLE_STRIP, 0, 4);
}

void image_t::draw(const graphics::textured_program_t& prog, point_t p, const rect_t &rect) const
{
    const float _w = fabsf(rect.right - rect.left), _h = fabsf(rect.bottom - rect.top), tx = tex.x / width, ty = tex.y / height;
    const float v[] = {
            p.x, p.y,  p.x + _w, p.y,
            p.x, p.y + _h, p.x + _w, p.y + _h
    };
    const float t[] = {
            rect.left * tx, rect.top * ty,  rect.right * tx, rect.top * ty,
            rect.left * tx, rect.bottom * ty, rect.right * tx, rect.bottom * ty
    };
    gl::ActiveTexture(gl::TEXTURE0);
    gl::BindTexture(gl::TEXTURE_2D, i);
    prog.position_vertex(v);
    prog.texture_vertex(t);
    gl::DrawArrays(gl::TRIANGLE_STRIP, 0, 4);
}

void image_t::draw(const graphics::textured_program_t& prog, const rect_t &rect) const
{
    const float _w = fabsf(rect.right - rect.left), _h = fabsf(rect.bottom - rect.top), tx = tex.x / width, ty = tex.y / height;
    const float v[] = {
            0, 0,  _w, 0,
            0, _h, _w, _h
    };
    const float t[] = {
            rect.left * tx, rect.top * ty,  rect.right * tx, rect.top * ty,
            rect.left * tx, rect.bottom * ty, rect.right * tx, rect.bottom * ty
    };
    gl::ActiveTexture(gl::TEXTURE0);
    gl::BindTexture(gl::TEXTURE_2D, i);
    prog.position_vertex(v);
    prog.texture_vertex(t);
    gl::DrawArrays(gl::TRIANGLE_STRIP, 0, 4);
}

void sprite_t::draw(const graphics::textured_program_t& prog) const
{
    const float v[] = {
            -center.x, -center.y,  size.x - center.x, -center.y,
            -center.x, size.y - center.y, size.x - center.x, size.y - center.y
    };
    const float t[] = {
            rect.left, rect.top,  rect.right, rect.top,
            rect.left, rect.bottom, rect.right, rect.bottom
    };
    gl::ActiveTexture(gl::TEXTURE0);
    gl::BindTexture(gl::TEXTURE_2D, image);
    prog.position_vertex(v);
    prog.texture_vertex(t);
    gl::DrawArrays(gl::TRIANGLE_STRIP, 0, 4);
}

void sprite_t::draw(const graphics::textured_program_t& prog, point_t p) const
{
    p.x -= center.x;
    p.y -= center.y;
    const float v[] = {
            p.x, p.y,  p.x + size.x, p.y,
            p.x, p.y + size.y, p.x + size.x, p.y + size.y
    };
    const float t[] = {
            rect.left, rect.top,  rect.right, rect.top,
            rect.left, rect.bottom, rect.right, rect.bottom
    };
    gl::ActiveTexture(gl::TEXTURE0);
    gl::BindTexture(gl::TEXTURE_2D, image);
    prog.position_vertex(v);
    prog.texture_vertex(t);
    gl::DrawArrays(gl::TRIANGLE_STRIP, 0, 4);
}
}
