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

#include <future>
#include "platform/display.hpp"
#include "top.hpp"
#include "application.hpp"
#include "drawable.hpp"
#include "draw_text.hpp"
#include "hsv.hpp"
#include "room_variant.hpp"
#include "room_controller.hpp"
#include "glass.hpp"

namespace
{
idle::controller room_ctrl;
overlay top;

}  // namespace

namespace isolation
{

bool application::execute_commands()
{
    window.event_loop_back(!update_display);

    if (!window.commands.size())
        return true;

    bool perform_load = false;

    constexpr auto log_prefix = "command::%s";

    for (const auto cmd : window.commands)
        switch (cmd)
        {
        case ::platform::command::InitWindow:
            LOGI(log_prefix, "InitWindow");
            perform_load = true;
            break;
        case ::platform::command::SaveState:
            LOGI(log_prefix, "SaveState");
            break;

        case ::platform::command::GainedFocus:
            LOGI(log_prefix, "GainedFocus");
            update_display = true;
            break;

        case ::platform::command::LostFocus:
            LOGI(log_prefix, "LostFocus");
            if (update_display)
                draw();
            update_display = false;
            break;

        case ::platform::command::GLCleanUp:
            LOGI(log_prefix, "GLCleanUp");
            update_display = false;
            if (pause)
            {
                pause->buffer.reset();
                pause->buffer_blur.reset();
            }
            top.gl.clean();
            window.terminate_display();
            break;

        case ::platform::command::CloseWindow:
            LOGI(log_prefix, "CloseWindow");
            top.gl.clean();
            window.terminate_display();
            top.request_shutdown();
            return false;

        case ::platform::command::PausePressed:
            LOGI(log_prefix, "PausePressed");
            room_ctrl.slumber();
            if (!pause)
                pause.emplace();
            break;
        }

    window.commands.clear();

    if (perform_load && !top.gl.setup_graphics()) {
        // throw idle::haiku("idle meant to load assets.", "But it crashed instead.");
        return false;
    }

    if (window.resize_request && std::chrono::system_clock::now() >= window.resize_request->tp)
    {
        resize_internal();
    }

    if (perform_load && !load()) {
        // throw idle::haiku("idle meant to load assets.", "But it crashed instead.");
        return false;
    }

    return !top.shutdown_was_requested();
}


void application::resize_internal()
{
    const auto& rq = *window.resize_request;
    top.gl.resize(rq.w, rq.h, rq.q, rq.r);

    window.resize_request.reset();

    room_ctrl.resize({
        static_cast<float>(top.gl.draw_size.x),
        static_cast<float>(top.gl.draw_size.y)
    });
}

namespace
{
GLint setup_buffer_frame(const graphics::render_buffer_t& rb, const violet::point2<int> internal_size, const idle::color_t& bg)
{
    GLint default_frame_buffer = 0;
    gl::GetIntegerv(gl::FRAMEBUFFER_BINDING, &default_frame_buffer);
    gl::BindFramebuffer(gl::FRAMEBUFFER, rb.buffer_frame);
    gl::Viewport(0, 0, internal_size.x, internal_size.y);
    gl::ClearColor(bg.r, bg.g, bg.b, 1);
    gl::Clear(gl::COLOR_BUFFER_BIT | gl::DEPTH_BUFFER_BIT);
    return default_frame_buffer;
}

class render_frame
{
    GLint default_frame_buffer = 0;
    ::platform::window& window;

public:
    render_frame(::platform::window& win)
        : default_frame_buffer(setup_buffer_frame(*top.gl.render_buffer, top.gl.internal_size, ::platform::background))
        , window(win)
    {}

    ~render_frame()
    {
        gl::BindFramebuffer(gl::FRAMEBUFFER, default_frame_buffer);
        gl::UseProgram(top.gl.render_program);
        gl::Viewport(0, 0, top.gl.screen_size.x, top.gl.screen_size.y);

        constexpr float v[] = {
                -1.f, -1.f,  1.f, -1.f,
                -1.f, 1.f, 1.f, 1.f
        };
        const float t[] = {
                0, 0,   top.gl.render_buffer->texture_w, 0,
                0, top.gl.render_buffer->texture_h, top.gl.render_buffer->texture_w, top.gl.render_buffer->texture_h
        };

        gl::ActiveTexture(gl::TEXTURE0);
        gl::BindTexture(gl::TEXTURE_2D, top.gl.render_buffer->texture);
        gl::VertexAttribPointer(top.gl.render_position_handle, 2, gl::FLOAT, gl::FALSE_, 0, v);
        gl::VertexAttribPointer(top.gl.render_texture_position_handle, 2, gl::FLOAT, gl::FALSE_, 0, t); //full_rect_texture
        gl::DrawArrays(gl::TRIANGLE_STRIP, 0, 4);

        window.buffer_swap();
    }
};

void draw_one_onto_the_other(const graphics::core& gl, const graphics::render_buffer_t& src)
{
    constexpr float v[] = {
            -1.f, -1.f,  1.f, -1.f,
            -1.f, 1.f, 1.f, 1.f
    };
    const float t[] = {
        0, 0,
        src.texture_w, 0,
        0, src.texture_h,
        src.texture_w, src.texture_h
    };
    gl::UseProgram(gl.render_program);
    gl::ActiveTexture(gl::TEXTURE0);
    gl::BindTexture(gl::TEXTURE_2D, src.texture);
    gl::VertexAttribPointer(gl.render_position_handle, 2, gl::FLOAT, gl::FALSE_, 0, v);
    gl::VertexAttribPointer(gl.render_texture_position_handle, 2, gl::FLOAT, gl::FALSE_, 0, t); //full_rect_texture
    gl::DrawArrays(gl::TRIANGLE_STRIP, 0, 4);
}

void draw_exit_splash(const graphics::core& gl, const float exit_splash_animation, const graphics::render_buffer_t& rb, const graphics::render_buffer_t& brb)
{
    gl.pnormal.use();
    gl.pnormal.set_identity();
    gl.pnormal.set_view_identity();
    const float v[] = {
        0, 0,
        static_cast<float>(gl.draw_size.x), 0,
        0, static_cast<float>(gl.draw_size.y),
        static_cast<float>(gl.draw_size.x), static_cast<float>(gl.draw_size.y)
    };
    const float t[] = {
        0, rb.texture_h,
        rb.texture_w, rb.texture_h,
        0, 0,
        rb.texture_w, 0
    };
    const float tb[] = {
        0, brb.texture_h,
        brb.texture_w, brb.texture_h,
        0, 0,
        brb.texture_w, 0
    };

    gl::ActiveTexture(gl::TEXTURE0);
    if (exit_splash_animation < .966f)
    {
        gl.pnormal.set_color({1, 1, 1, 1 - exit_splash_animation});
        gl::BindTexture(gl::TEXTURE_2D, rb.texture);
        gl.pnormal.position_vertex(v);
        gl.pnormal.texture_vertex(t);
        gl::DrawArrays(gl::TRIANGLE_STRIP, 0, 4);
    }

    gl.pnormal.set_color({1, 1 - exit_splash_animation * .2f, 1 - exit_splash_animation * .1f, exit_splash_animation * .668f});
    gl::BindTexture(gl::TEXTURE_2D, brb.texture);
    gl.pnormal.position_vertex(v);
    gl.pnormal.texture_vertex(tb);
    gl::DrawArrays(gl::TRIANGLE_STRIP, 0, 4);

    // gl.pfill.use();
    // gl.pfill.set_identity();
    // gl.pfill.set_view_identity();

    const float yshift = (1 - exit_splash_animation) * 20;
    const float topy = gl.draw_size.y / 2 - 60.f;
    const float v2[] {
        gl.draw_size.x * .01f, topy,
        gl.draw_size.x * .01f, topy + 120,
        gl.draw_size.x * .4f, topy,
        gl.draw_size.x * .4f, topy + 120,
        gl.draw_size.x * .6f, topy,
        gl.draw_size.x * .6f, topy + 120,
        gl.draw_size.x * .99f, topy,
        gl.draw_size.x * .99f, topy + 120
    };

    constexpr float t2[] {
        0, 0,
        0, 0,
        0, 1,
        0, 1,
        0, 1,
        0, 1,
        0, 0,
        0, 0
    };

    // gl.pnormal.set_color(0, 0, 0, .81f * powf(exit_splash_animation, 3)); // .2f
    // gl::ActiveTexture(gl::TEXTURE0);
    // gl::BindTexture(gl::TEXTURE_2D, gl.image_id_fade);
    // gl.pnormal.position_vertex(v2);
    // gl.pnormal.texture_vertex(t2);
    // gl::DrawArrays(gl::TRIANGLE_STRIP, 0, 8);

    const auto rect = idle::rect_t(gl.draw_size.x / 2 - 220.f, gl.draw_size.y / 2 - 60.f, gl.draw_size.x / 2 + 220.f, gl.draw_size.y / 2 + 60.f);
    gl.ptext.use();
    gl.ptext.set_color(1, .133f, .196f, exit_splash_animation);

    idle::draw_text<idle::TextAlign::Center>(gl, "paused",
                        idle::point_t(gl.draw_size.x / 2.f, rect.top + 10.f + yshift), 55);

    idle::draw_text<idle::TextAlign::Center, idle::TextAlign::Center>(gl, "resume",
                        idle::point_t(gl.draw_size.x / 2.f, rect.bottom - 35.f + yshift), 14 * exit_splash_animation + 16);
}

void wait_one_frame_with_skipping(std::chrono::steady_clock::time_point& new_time)
{
    using namespace std::chrono_literals;
    constexpr auto minimum_elapsed_duration = std::chrono::duration_cast<
                        std::chrono::steady_clock::time_point::duration>(1.0s / APPLICATION_FPS);

    if (std::chrono::steady_clock::now() - new_time < 1.0s)
    {
        new_time += minimum_elapsed_duration;
        std::this_thread::sleep_until(new_time);
    }
    else
    {
        new_time += minimum_elapsed_duration * APPLICATION_FPS;
    }
}

struct loading_animation
{
    unsigned int iterator = (-1);
    idle::image_t picture, background;
    float alpha = 0;
    bool loaded = false;

    template<typename Im1, typename Im2>
    loading_animation(Im1&& i1, Im2&& i2)
        : picture(std::forward<Im1>(i1)), background(std::forward<Im2>(i2))
    {
    }

    void draw()
    {
        auto bgsz = background.get_size<float>();
        const float bgsc = std::max(static_cast<float>(top.gl.draw_size.y) / bgsz.y, static_cast<float>(top.gl.draw_size.x) / bgsz.x);
        constexpr idle::rect_t emily{0, 0, 398, 432}, /* text_loading{0, 512 - 70, 360, 512},*/ text_cp{450, 0, 512, 512};
        const float hdiv = top.gl.draw_size.y / 6.f;
        constexpr auto rbw = idle::rainbow_from_saturation(.96f);

        top.gl.pnormal.use();
        top.gl.pnormal.set_color(1, 1, 1, 1);
        top.gl.pnormal.set_transform(idle::mat4x4_t::translate((top.gl.draw_size.x - bgsz.x) * .5f, (top.gl.draw_size.y - bgsz.y) * .5f) * idle::mat4x4_t::scale(bgsc, bgsc, {top.gl.draw_size.x / 2.f, top.gl.draw_size.y / 2.f}));
        background.draw(top.gl.pnormal);

        if (alpha < .998f) {
            top.gl.pnormal.set_transform(idle::mat4x4_t::scale(sinf(alpha * F_TAU_4) *.5f + .5f, idle::point_t((emily.right - emily.left) / 2, (emily.bottom - emily.top) / 2))
                * idle::mat4x4_t::scale(.6f) * idle::mat4x4_t::translate(top.gl.draw_size.x / 2 - 80.f, 25.f));
            alpha += .025f;
            top.gl.pnormal.set_color(1, 1, 1, alpha * alpha);
        }
        else
        top.gl.pnormal.set_transform(idle::mat4x4_t::scale(.6f) * idle::mat4x4_t::translate(top.gl.draw_size.x / 2 - 80.f, 25.f));
        picture.draw(top.gl.pnormal, emily);

        top.gl.pnormal.set_color(1, 1, 1, 1);
        top.gl.pnormal.set_identity();
        top.gl.pnormal.set_view_identity();

        top.gl.pfill.use();
        top.gl.pfill.set_identity();
        top.gl.pfill.set_view_identity();

        for (int i = 0; i < 6; ++i) {
            top.gl.pfill.set_color(rbw[i], .51f);
            float ht = hdiv * i + iterator;
            if (ht >= static_cast<float>(top.gl.draw_size.y))
                ht -= static_cast<float>(top.gl.draw_size.y);
            else if (ht + hdiv >= static_cast<float>(top.gl.draw_size.y))
                idle::fill_rectangle(top.gl.pfill, idle::rect_t(0, ht - static_cast<float>(top.gl.draw_size.y), 30.f, ht + hdiv - static_cast<float>(top.gl.draw_size.y)));
            idle::fill_rectangle(top.gl.pfill, idle::rect_t(0, ht, 30.f, ht + hdiv));
        }
        if (++iterator >= static_cast<unsigned>(top.gl.draw_size.y))
            iterator = 0;

        top.gl.pnormal.use();
        top.gl.pnormal.set_color(1, 1, 1, 1);
        top.gl.pnormal.set_transform(idle::mat4x4_t::rotate(violet::degtorad<float>(90)) * idle::mat4x4_t::scale(.4f) * idle::mat4x4_t::translate(top.gl.draw_size.x - 1, 1));
        picture.draw(top.gl.pnormal, text_cp);
    }
};

}


void application::draw()
{
    if (pause)
    {
        if (!pause->buffer || !pause->buffer_blur)
        {
            pause->buffer.emplace(top.gl, 1);
            pause->buffer_blur.emplace(top.gl, 4);
            const auto def = setup_buffer_frame(*pause->buffer, top.gl.internal_size, platform::background);
            room_ctrl.draw_frame(top.gl);
            setup_buffer_frame(*pause->buffer_blur, top.gl.internal_size / 4, platform::background);
            draw_one_onto_the_other(top.gl, *pause->buffer);
            gl::BindFramebuffer(gl::FRAMEBUFFER, def);
        }
        render_frame _frame_buffer_{window};
        draw_exit_splash(top.gl, pause->animation, *pause->buffer, *pause->buffer_blur);
    }
    else
    {
        render_frame _frame_buffer_{window};
        room_ctrl.draw_frame(top.gl);
    }
}

int application::real_main()
{
#ifdef __clang__
    LOGD("%s", "### CLANG " __clang_version__);
#elif __GNUC__
    LOGD("### GCC %d.%d", __GNUC__, __GNUC_MINOR__);
#endif

    auto app_time = std::chrono::steady_clock::now();

    while (execute_commands() && room_ctrl.execute_pending_room_change(top))
    {
        if (pause)
        {
            if (window.cursor.single_press)
            {
                const auto p = top.get_pointer().pos;
                if (fabsf(p.x - top.gl.draw_size.x / 2) < 150.f
                        && fabsf(p.y - (top.gl.draw_size.y / 2 + 30.f)) < 60.f)
                {
                    pause.reset();
                    pointer.clear(window.cursor);
                    room_ctrl.wake(top, app_time);
                    continue;
                }
            }
            pause->animation += (1 - pause->animation) / 20;
        }

        if (window.has_gl_context())
        {
            if (update_display)
                draw();

            idle::image_t::load_topmost_queued_picture();
        }

        pointer.update(window.cursor);
        top.set_pointer(window.cursor);

        wait_one_frame_with_skipping(app_time);
    }
    return 0;
}

bool application::load()
{
    using namespace std::chrono_literals;
    constexpr auto minimum_elapsed = 1.0s / (APPLICATION_FPS / 2 + 5);

    if (room_ctrl.get_crashed())
        return false;

    LOGI("Asset refresh requested");

    loading_animation la {
        idle::image_t::load_from_assets_immediate("path4368.png"),
        idle::image_t::load_from_assets_immediate("space-1.png")
    };

    room_ctrl.slumber();
    room_ctrl.kill_during_sleep();

    la.iterator = top.gl.draw_size.y - top.gl.draw_size.y / 12;

    std::chrono::time_point<std::chrono::steady_clock> lt = std::chrono::steady_clock::now();
    std::promise<bool> loader_callback;
    auto loader_result = loader_callback.get_future();

    std::thread loader_thread {
            [this] (std::promise<bool> promise)
            {
                promise.set_value(top.db.load_everything(window, top));
            },
            std::move(loader_callback)
        };

    while (loader_result.wait_for(std::chrono::milliseconds(5)) != std::future_status::ready)
    {
        if (!execute_commands())
        {
            LOGE("Loader called shut down.");
            loader_thread.join();
            return false;
        }

        if (window.has_gl_context())
        {
            idle::image_t::load_topmost_queued_picture();

            if (update_display && std::chrono::steady_clock::now() - lt > minimum_elapsed)
            {
                lt += std::chrono::duration_cast<std::chrono::steady_clock::time_point::duration>(minimum_elapsed);
                render_frame _frame_buffer_{window};
                la.draw();
            }
        }
    }

    if (!loader_result.get())
        return false;

    loader_thread.join();

    // room_ctrl.default_room_if_none_set();

    return true;
}

}  // namespace isolation

