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

#include <optional>
#include <math.hpp>
#include <log.hpp>
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


void printGLString(const char *name, GLenum s);

bool assertGlErrors();

#ifndef NDEBUG
void _checkGlError_(const char* op, const char *const _file, const int _line);

#define checkGlError(what) _checkGlError_((what), __FILE__, __LINE__)
#else
#define checkGlError(x) ((void)0)
#endif

namespace graphics
{
struct core;

struct render_buffer_t
{
    GLuint buffer_frame = 0, buffer_depth = 0, texture = 0;
    GLfloat texture_w, texture_h;

    render_buffer_t(const core&, int divider);

    ~render_buffer_t();
};

struct core
{
    textured_program_t pnormal;
    double_vertex_program_t pdouble;
    program_t pfill;
    text_program_t ptext;
    fullbg_program_t pfullbg;

    idle::mat4x4_t projectionMatrix;

    GLuint image_id_fade = 0, image_id_noise = 0;
    GLuint render_texture_position_handle = 0, render_program = 0, render_position_handle = 0;
    GLint render_quality = gl::LINEAR;
    std::optional<font_t> font;
    std::optional<render_buffer_t> render_buffer;
    math::point2<int> draw_size{0, 0}, screen_size{0, 0}, internal_size{0, 0};
    math::point2<float> translate_vector;


    bool all_programs_are_functional() const;

    void copy_projection_matrix() const;

    bool setup_graphics();

    void resize(int window_width, int window_height, int quality, int resolution);

    render_buffer_t new_render_buffer(int divider) const;

    void clean();

};


} // namespace graphics
