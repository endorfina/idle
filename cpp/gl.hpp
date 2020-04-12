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

#include <atomic>
#include <optional>
#include <math.hpp>
#include <log.hpp>

#include "platform/pointer.hpp"
#include "platform/opengl_core_adaptive.hpp"
#include "gl_programs.hpp"
#include "fonts.hpp"

#define TEMPLATE_CHECK_METHOD(fname) template<typename T, typename = void>\
    struct has_##fname##_method : std::false_type {};\
    template<typename T>\
    struct has_##fname##_method<\
        T,\
        std::enable_if_t<\
            std::is_member_function_pointer_v<decltype(&T::fname)>\
        >\
    > : public std::true_type {}

#define TEMPLATE_CHECK_MEMBER(fname) template<typename T, typename = void>\
    struct has_##fname##_member : std::false_type {};\
    template<typename T>\
    struct has_##fname##_member<\
        T,\
        std::enable_if_t<\
            std::is_member_pointer_v<decltype(&T::fname)>\
        >\
    > : public std::true_type {}


namespace graphics
{
bool assert_opengl_errors();

struct core;

struct render_buffer_t
{
    GLuint buffer_frame = 0, buffer_depth = 0, texture = 0;
    GLfloat texture_w, texture_h;

    render_buffer_t(math::point2<int> size, GLint quality);

    render_buffer_t(render_buffer_t&&)=delete;

    render_buffer_t(const render_buffer_t&)=delete;

    ~render_buffer_t();
};

struct render_program_t
{
    GLuint program = 0;

private:
    GLuint texture_position_handle = 0, position_handle = 0;

public:
    void draw_buffer(const render_buffer_t& source) const;

    void use() const;

    void prepare();
};

struct blur_render_program_t : render_program_t
{
private:
    GLint direction_handle = 0, resolution_handle = 0, radius_handle = 0;

public:
    void prepare();

    void set_radius(GLfloat x) const;

    void set_resolution(GLfloat x) const;

    void set_direction(GLfloat x, GLfloat y) const;
};

struct masked_render_program_t : render_program_t
{
private:
    GLint mask_offset_handle = 0;

public:
    void prepare();

    void set_offsets(GLfloat ratio, GLfloat buffer_height) const;
};

struct core
{
    struct program_container_t
    {
        render_program_t render_final;
        masked_render_program_t render_masked;
        blur_render_program_t render_blur;

        textured_program_t normal;
        double_vertex_program_t shift;
        program_t fill;
        text_program_t text;
        fullbg_program_t fullbg;

    } prog;

    GLuint image_id_fade = 0, image_id_noise = 0;
    GLint render_quality = gl::LINEAR;
    std::optional<font_t> font;
    std::optional<render_buffer_t> render_buffer, render_buffer_masked;
    math::point2<int> draw_size{0, 0}, screen_size{0, 0}, internal_size{0, 0};
    math::point2<float> translate_vector;

    bool shutdown_was_requested = false;
    std::atomic<platform::pointer> pointer;

    void copy_projection_matrix(const idle::mat4x4_t&) const;

    bool setup_graphics();

    void resize(int window_width, int window_height);

    void new_render_buffer(std::optional<render_buffer_t>& opt, int divider = 1) const;

    void clean();

};


} // namespace graphics
