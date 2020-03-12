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

#include <vector>
#include <zlib.hpp>
#include <math.hpp>
#include "gl.hpp"
#include <embedded_shaders.hpp>

// void printGLString(const char *name, GLenum s)
// {
//     LOGI("GL %s = %s\n", name, reinterpret_cast<const char *>(gl::GetString(s)));
// }

namespace graphics
{
namespace
{

#ifndef NDEBUG
void detailed_report_opengl_errors(const char* op, const char *const file, const int line)
{
    if (assert_opengl_errors())
    {
        LOGE("Reported \'%s\' at %s:%i", op, file, line);
    }
}

#define report_opengl_errors(what) detailed_report_opengl_errors((what), __FILE__, __LINE__)
#else
#define report_opengl_errors(x) ((void)0)
#endif

GLuint load_shader(const GLenum shaderType, const char* const pSource)
{
    GLuint shader = gl::CreateShader(shaderType);
    if (shader)
    {
        gl::ShaderSource(shader, 1, &pSource, nullptr);
        gl::CompileShader(shader);
        GLint compiled = 0;
        gl::GetShaderiv(shader, gl::COMPILE_STATUS, &compiled);

        if (!compiled)
        {
            GLint infoLen = 0;
            gl::GetShaderiv(shader, gl::INFO_LOG_LENGTH, &infoLen);

            if (infoLen)
            {
                char buf[infoLen];
                gl::GetShaderInfoLog(shader, infoLen, nullptr, buf);
                LOGE("Could not compile shader %d:\n%s", shaderType, buf);
                gl::DeleteShader(shader);
                shader = 0;
            }
        }
    }
    return shader;
}

std::optional<GLuint> create_program(const char* const pVertexSource, const char* const pFragmentSource)
{
    GLuint vertexShader = load_shader(gl::VERTEX_SHADER, pVertexSource);
    if (!vertexShader)
        return {};

    GLuint pixelShader = load_shader(gl::FRAGMENT_SHADER, pFragmentSource);
    if (!pixelShader)
        return {};

    if (GLuint program = gl::CreateProgram())
    {
        gl::AttachShader(program, vertexShader);
        gl::AttachShader(program, pixelShader);
        gl::LinkProgram(program);
        GLint linkStatus = gl::FALSE_;
        gl::GetProgramiv(program, gl::LINK_STATUS, &linkStatus);

        if (linkStatus != gl::TRUE_)
        {
            GLint bufLength = 0;
            gl::GetProgramiv(program, gl::INFO_LOG_LENGTH, &bufLength);
            if (bufLength)
            {
                char buf[bufLength];
                gl::GetProgramInfoLog(program, bufLength, nullptr, buf);
                LOGE("Could not link program:\n%s", buf);
            }
            gl::DeleteProgram(program);
            return {};
        }
        return { program };
    }
    return {};
}

struct shader_compiler
{
    using buffer_t = std::vector<unsigned char>;

    shader_compiler(std::string_view view, const size_t size_check)
        : buffer(idle::zlib<buffer_t>(view, false, true))
    {
        if (!buffer || buffer->size() != size_check)
        {
            buffer.reset();
        }
    }

private:
    const char * data(const unsigned int addr) const
    {
        return reinterpret_cast<const char*>(buffer->data()) + addr;
    }

public:
    GLuint compile(const unsigned int v, const unsigned int f)
    {
        if (!has_failed())
        {
            if (const auto r = create_program(data(v), data(f)))
            {
                return *r;
            }
            else
            {
                buffer.reset();
            }
        }
        return 0;
    }

private:
    std::optional<buffer_t> buffer;

public:
    bool has_failed() const
    {
        return !buffer;
    }
};

}  // namespace

bool core::setup_graphics()
{
    gl::Disable(gl::CULL_FACE);
    gl::Disable(gl::DEPTH_TEST);
    gl::BlendFunc(gl::SRC_ALPHA, gl::ONE_MINUS_SRC_ALPHA);
    gl::Enable(gl::BLEND);

    {
        shader_compiler sc{ shaders::get_data(), shaders::source_size_uncompressed };

        render_program = sc.compile(shaders::source_pos_renderv, shaders::source_pos_renderf);
        pnormal.pid = sc.compile(shaders::source_pos_normv, shaders::source_pos_normf);
        pdouble.pid = sc.compile(shaders::source_pos_doublev, shaders::source_pos_normf);
        pfill.pid = sc.compile(shaders::source_pos_solidv, shaders::source_pos_solidf);
        ptext.pid = sc.compile(shaders::source_pos_textv, shaders::source_pos_textf);
        pfullbg.pid = sc.compile(shaders::source_pos_solidv, shaders::source_pos_fullbgf);


        if (sc.has_failed())
        {
            LOGE("Shader compilation failed.\n" "How unfortunate.");
            return false;
        }
    }

    render_position_handle = static_cast<GLuint>(gl::GetAttribLocation(render_program, "vPos"));
    render_texture_position_handle = static_cast<GLuint>(gl::GetAttribLocation(render_program, "aUV"));

    pnormal.collect_variables();
    pfill.collect_variables();
    ptext.collect_variables();
    pdouble.collect_variables();
    pfullbg.collect_variables();

    pfullbg.use();
    pfullbg.set_offset(0);

    pdouble.use();
    pdouble.set_interpolation(0);

    gl::UseProgram(render_program);
    gl::EnableVertexAttribArray(render_position_handle);
    gl::EnableVertexAttribArray(render_texture_position_handle);

    pnormal.prepare();
    pfill.prepare();
    ptext.prepare();
    pdouble.prepare();
    pfullbg.prepare();
    pnormal.use();

#ifdef DEBUG
    // Check openGL on the system
    constexpr std::pair<GLenum, const char *> opengl_info[] {
        { gl::VENDOR, "Vendor" },
        { gl::RENDERER, "Renderer" },
        { gl::VERSION, "Version" }
        //, gl::EXTENSIONS
    };
    for (const auto [name, label] : opengl_info) {
        LOGI("OpenGL %s: %s", label, gl::GetString(name));
    }
#endif
    return true;
}

void core::resize(const int window_width, const int window_height, const int quality, int resolution)
{
    LOGI("Changing resolution to %i x %i", window_width, window_height);

    if (window_width < window_height)
    {
        draw_size.x = 360;
        draw_size.y = (draw_size.x * window_height) / window_width;
    }
    else
    {
        draw_size.y = 360;
        draw_size.x = (draw_size.y * window_width) / window_height;
    }

    screen_size.x = window_width;
    screen_size.y = window_height;

    if (resolution < 0 && internal_size.y == 0)
    {
        resolution = 720;
        internal_size.y = resolution;
    }
    else {
        resolution = internal_size.y;
    }

    internal_size.x = (resolution * window_width) / window_height;

    if (quality >= 0)
        render_quality = quality;

    translate_vector.x = static_cast<float>(draw_size.x) / static_cast<float>(window_width);
    translate_vector.y = static_cast<float>(draw_size.y) / static_cast<float>(window_height);

    if (!all_programs_are_functional()) return;

    copy_projection_matrix(idle::mat4x4_t::orthof_static<-1, 1>(0, draw_size.x, 0, draw_size.y));

    pfullbg.use();
    pfullbg.set_resolution(static_cast<float>(window_width), static_cast<float>(window_height));
    render_buffer.emplace(*this, 1);
}


void program_t::set_transform(const idle::mat4x4_t& f) const
{
    gl::UniformMatrix4fv(model_handle, 1, gl::FALSE_, static_cast<const GLfloat*>(f));
}

void program_t::set_transform(const idle::mat4x4_noopt_t& f) const
{
    gl::UniformMatrix4fv(model_handle, 1, gl::FALSE_, static_cast<const GLfloat*>(f));
}

void program_t::set_identity(void) const
{
    gl::UniformMatrix4fv(model_handle, 1, gl::FALSE_, static_cast<const GLfloat*>(idle::mat4x4_t::identity()));
}

void program_t::set_view_transform(const idle::mat4x4_t& f) const
{
    gl::UniformMatrix4fv(view_handle, 1, gl::FALSE_, static_cast<const GLfloat*>(f));
}

void program_t::set_view_transform(const idle::mat4x4_noopt_t& f) const
{
    gl::UniformMatrix4fv(view_handle, 1, gl::FALSE_, static_cast<const GLfloat*>(f));
}

void program_t::set_view_identity(void) const
{
    gl::UniformMatrix4fv(view_handle, 1, gl::FALSE_, static_cast<const GLfloat*>(idle::mat4x4_t::identity()));
}

void program_t::use() const
{
    gl::UseProgram(pid);
}

void program_t::set_color(const idle::color_t& c) const
{
    gl::Uniform4f(color_handle, c.r, c.g, c.b, c.a);
}

void program_t::set_color(GLfloat r, GLfloat g, GLfloat b, GLfloat a) const
{
    gl::Uniform4f(color_handle, r, g, b, a);
}

void program_t::set_color(const idle::color_t& c, GLfloat custom_alpha) const
{
    gl::Uniform4f(color_handle, c.r, c.g, c.b, custom_alpha);
}

void program_t::position_vertex(const GLfloat *f) const
{
    gl::VertexAttribPointer(position_handle, 2, gl::FLOAT, gl::FALSE_, 0, f);
}

void textured_program_t::texture_vertex(const GLfloat *f) const
{
    gl::VertexAttribPointer(texture_position_handle, 2, gl::FLOAT, gl::FALSE_, 0, f);
}

void double_vertex_program_t::destination_vertex(const GLfloat *f) const
{
    gl::VertexAttribPointer(destination_handle, 2, gl::FLOAT, gl::FALSE_, 0, f);
}

void text_program_t::set_text_offset(const GLfloat x, const GLfloat y) const
{
    gl::Uniform2f(font_offset_handle, x, y);
}

void double_vertex_program_t::set_interpolation(const GLfloat x) const
{
    gl::Uniform1f(interpolation_handle, x);
}

void fullbg_program_t::set_offset(const GLfloat x) const
{
    gl::Uniform1f(offset_handle, x);
}

void fullbg_program_t::set_resolution(const GLfloat w, const GLfloat h) const
{
    gl::Uniform2f(resolution_handle, w, h);
}


void program_t::collect_variables()
{
    position_handle = static_cast<GLuint>(gl::GetAttribLocation(pid, "vPos"));
    model_handle = gl::GetUniformLocation(pid, "uMM");
    view_handle = gl::GetUniformLocation(pid, "uVM");
    color_handle = gl::GetUniformLocation(pid, "uCol");
    report_opengl_errors("program_t::collect_variables()");
}

void textured_program_t::collect_variables()
{
    program_t::collect_variables();
    texture_position_handle = static_cast<GLuint>(gl::GetAttribLocation(pid,"aUV"));
}

void text_program_t::collect_variables()
{
    textured_program_t::collect_variables();
    font_offset_handle = gl::GetUniformLocation(pid, "uOf");
    report_opengl_errors("text_program_t::collect_variables()");
}

void double_vertex_program_t::collect_variables()
{
    textured_program_t::collect_variables();
    destination_handle = static_cast<GLuint>(gl::GetAttribLocation(pid,"vDest"));
    interpolation_handle = gl::GetUniformLocation(pid, "uIv");
    report_opengl_errors("double_vertex_program_t::collect_variables()");
}

void fullbg_program_t::collect_variables()
{
    program_t::collect_variables();
    offset_handle = gl::GetUniformLocation(pid, "uI");
    resolution_handle = gl::GetUniformLocation(pid, "uR");
    report_opengl_errors("fullbg_program_t::collect_variables()");
}


void program_t::prepare() const
{
    use();
    set_identity();
    set_view_identity();
    gl::EnableVertexAttribArray(position_handle);
    report_opengl_errors("program_t::prepare()");
}

void textured_program_t::prepare() const
{
    program_t::prepare();
    gl::EnableVertexAttribArray(texture_position_handle);
    report_opengl_errors("textured_program_t::prepare()");
}

void double_vertex_program_t::prepare() const
{
    textured_program_t::prepare();
    LOGD("Destination handle %u", destination_handle);
    gl::EnableVertexAttribArray(destination_handle);
    report_opengl_errors("double_vertex_program_t::prepare()");
}



bool core::all_programs_are_functional() const
{
    return pnormal.pid && pdouble.pid && pfill.pid && ptext.pid && pfullbg.pid;
}

static void set_projection_matrix(const program_t& prog, const idle::mat4x4_t& mat)
{
    prog.use();
    gl::UniformMatrix4fv(gl::GetUniformLocation(prog.pid, "uPM"), 1, gl::FALSE_, static_cast<const GLfloat*>(mat));
}

void core::copy_projection_matrix(const idle::mat4x4_t& projectionMatrix) const
{
    set_projection_matrix(pnormal, projectionMatrix);
    set_projection_matrix(pdouble, projectionMatrix);
    set_projection_matrix(pfill, projectionMatrix);
    set_projection_matrix(ptext, projectionMatrix);
    set_projection_matrix(pfullbg, projectionMatrix);
}

render_buffer_t::~render_buffer_t()
{
    LOGD("Destroying render buffer [fr:%i/tex:%i/dep:%i]", buffer_frame, texture, buffer_depth);
    gl::DeleteFramebuffers(1, &buffer_frame);
    gl::DeleteTextures(1, &texture);
    gl::DeleteRenderbuffers(1, &buffer_depth);
}

render_buffer_t::render_buffer_t(const core& context, const int divider)
{
    const auto tex_size = context.internal_size / divider;

    gl::GenFramebuffers(1, &buffer_frame);
    gl::GenTextures(1, &texture);
    gl::GenRenderbuffers(1, &buffer_depth);

    report_opengl_errors("render_buffer_t::render_buffer_t");

    LOGD("Creating render buffer [fr:%i/tex:%i/dep:%i]", buffer_frame, texture, buffer_depth);

    gl::BindTexture(gl::TEXTURE_2D, texture);
    gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_MAG_FILTER, context.render_quality);
    gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_MIN_FILTER, context.render_quality);
    gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_WRAP_S, gl::CLAMP_TO_EDGE);
    gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_WRAP_T, gl::CLAMP_TO_EDGE);
    int u = 256, v = 256;
    while (u < tex_size.x) u *= 2;
    while (v < tex_size.y) v *= 2;
    texture_w = static_cast<GLfloat>(tex_size.x) / static_cast<GLfloat>(u);
    texture_h = static_cast<GLfloat>(tex_size.y) / static_cast<GLfloat>(v);

    gl::TexImage2D(gl::TEXTURE_2D, 0, gl::RGBA, u, v, 0, gl::RGBA, gl::UNSIGNED_BYTE, nullptr);
    gl::BindRenderbuffer(gl::RENDERBUFFER, buffer_depth);
    gl::RenderbufferStorage(gl::RENDERBUFFER, gl::DEPTH_COMPONENT16, u, v);
    gl::BindFramebuffer(gl::FRAMEBUFFER, buffer_frame);
    gl::FramebufferTexture2D(gl::FRAMEBUFFER, gl::COLOR_ATTACHMENT0, gl::TEXTURE_2D, texture, 0);
    gl::FramebufferRenderbuffer(gl::FRAMEBUFFER, gl::DEPTH_ATTACHMENT, gl::RENDERBUFFER, buffer_depth);

    report_opengl_errors("render_buffer_t::render_buffer_t");

    gl::BindTexture(gl::TEXTURE_2D, 0);
    gl::BindRenderbuffer(gl::RENDERBUFFER, 0);
    gl::BindFramebuffer(gl::FRAMEBUFFER, 0);
}

void core::clean()
{
    font.reset();
    render_buffer.reset();
}

unique_texture::unique_texture(const GLuint val) : value{val}
{
}

unique_texture::unique_texture(unique_texture&& other) : value{other.value}
{
    other.value = GLuint(-1);
}

unique_texture& unique_texture::operator=(unique_texture&& other)
{
    value = other.value;
    other.value = GLuint(-1);
    return *this;
}

unique_texture::~unique_texture()
{
    if (value != GLuint(-1))
    {
        LOGD("Destroying unique texture #%u", value);
        gl::DeleteTextures(1, &value);
    }
}

bool assert_opengl_errors()
{
    if (auto error = gl::GetError(); error != gl::NO_ERROR_) {
        do {
            LOGE("GL Error \t0x%04x", error);
            error = gl::GetError();
        } while (error != gl::NO_ERROR_);
        return true;
    }
    return false;
}

} // namespace graphics
