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

#include <cstdlib>
#include <cassert>
#include <chrono>
#include <thread>
#include <memory>
#include <vector>

#include <EGL/egl.h>
#include "opengl_core_adaptive.hpp"
#include "context.hpp"
#include "asset_access.hpp"

namespace platform
{
namespace
{

struct egl_display
{
    struct android_app * android;
    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;

    egl_display() = delete;

    static const egl_display& cast(const context::data_t& data) noexcept
    {
        return *std::launder(reinterpret_cast<const egl_display*>(data));
    }

    static egl_display& cast(context::data_t& data) noexcept
    {
        return *std::launder(reinterpret_cast<egl_display*>(data));
    }
};

static_assert(sizeof(egl_display) <= sizeof(context::data_t));

std::optional<resize_request_t> create_window(egl_display& egl) noexcept
{
    // initialize OpenGL ES and EGL
    EGLint w, h;

    {
        EGLint format;
        EGLSurface get_surface;
        EGLContext get_context;
        EGLDisplay get_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

        eglInitialize(get_display, 0, 0);

        /* Here, the application chooses the configuration it desires. In this
        * sample, we have a very simplified selection process, where we pick
        * the first EGLConfig that matches our criteria */

        const auto sconfig = [get_display]()->EGLConfig
        {
            /* all higher than 4 are OK with me */
            static constexpr EGLint attributes[]
            {
                EGL_RED_SIZE, 4,
                EGL_GREEN_SIZE, 4,
                EGL_BLUE_SIZE, 4,
                EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
                EGL_NONE
            };

            EGLint numConfigs;
            eglChooseConfig(get_display, attributes, nullptr, 0, &numConfigs);
            if (!numConfigs)
                return nullptr;

            std::vector<EGLConfig> supportedConfigs(numConfigs);
            eglChooseConfig(get_display, attributes, supportedConfigs.data(), supportedConfigs.size(), &numConfigs);
            if (!numConfigs)
                return nullptr;

            for (auto& cfg : supportedConfigs)
            {
                EGLint r, g, b, d;
                if (eglGetConfigAttrib(get_display, cfg, EGL_RED_SIZE, &r) &&
                    eglGetConfigAttrib(get_display, cfg, EGL_GREEN_SIZE, &g) &&
                    eglGetConfigAttrib(get_display, cfg, EGL_BLUE_SIZE, &b) &&
                    eglGetConfigAttrib(get_display, cfg, EGL_DEPTH_SIZE, &d))
                {
                    if (r + g + b <= 8 * 3 && d > 0)
                    {
                        LOGI("Context [r:%i, g:%i, b:%i, d:%i]", r, g, b, d);
                        return cfg;
                    }
                }
            }
            LOGW("Picked the first config");
            return supportedConfigs.front();
        }();

        /* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
        * guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
        * As soon as we picked a EGLConfig, we can safely reconfigure the
        * ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
        eglGetConfigAttrib(get_display, sconfig, EGL_NATIVE_VISUAL_ID, &format);

        ANativeWindow_setBuffersGeometry(egl.android->window, 0, 0, format);

        get_surface = eglCreateWindowSurface(get_display, sconfig, egl.android->window, nullptr);
        EGLint attrib_list[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
        get_context = eglCreateContext(get_display, sconfig, NULL, attrib_list);

        if (eglMakeCurrent(get_display, get_surface, get_surface, get_context) == EGL_FALSE) {
            LOGE("Unable to eglMakeCurrent");
            return {};
        }

        eglQuerySurface(get_display, get_surface, EGL_WIDTH, &w);
        eglQuerySurface(get_display, get_surface, EGL_HEIGHT, &h);

        egl.display = get_display;
        egl.context = get_context;
        egl.surface = get_surface;
    }

    if (static const gl::exts::LoadTest glTest = gl::sys::LoadFunctions(); !glTest)
    {
        LOGE("Failed to load crucial OpenGL functions");
        return {};
    }
    else if (const auto amt = glTest.GetNumMissing())
    {
        LOGE("Number of functions that failed to load: %i.", amt);
    }

    return {{ static_cast<unsigned>(w), static_cast<unsigned>(h) }};
}

int32_t android_handle_input(android_app * app, AInputEvent* event) noexcept
{
    auto& win = *reinterpret_cast<context*>(app->userData);

    switch (AInputEvent_getType(event))
    {
    case AINPUT_EVENT_TYPE_MOTION:
        win.cursor.pos.x = AMotionEvent_getX(event, 0);
        win.cursor.pos.y = AMotionEvent_getY(event, 0);

        switch (AKeyEvent_getAction(event))
        {
        case AMOTION_EVENT_ACTION_DOWN:
            win.cursor.pressed = true;
            break;
        case AMOTION_EVENT_ACTION_UP:
            win.cursor.pressed = false;
            break;
        }

        win.cursor_update = true;
        break;

    case AINPUT_EVENT_TYPE_KEY:
        switch (AKeyEvent_getAction(event))
        {
        case AKEY_EVENT_ACTION_DOWN:
        {
            int key = AKeyEvent_getKeyCode(event);
            if (key == AKEYCODE_BACK) {
                win.commands.insert(command::pause_pressed);
                return 1;
            }
        }
        break;
        }
    }
    return 0;
}

void android_handle_command(struct android_app* app, const int32_t cmd) noexcept
{
    auto& win = *reinterpret_cast<context*>(app->userData);

    switch (cmd) {
        case APP_CMD_SAVE_STATE:
            win.commands.insert(command::save_state);
            break;

        case APP_CMD_INIT_WINDOW:
            // The window is being shown, get it ready.
            if (!!app->window && (win.resize_request = create_window(egl_display::cast(win.data))))
            {
                win.commands.insert(command::init_window);
            }
            else
            {
                LOGE("Cannot create the window!");
                win.commands.insert(command::close_window);
            }
            break;

        case APP_CMD_TERM_WINDOW:
            win.commands.insert(command::gl_clean_up);
            break;

        case APP_CMD_GAINED_FOCUS:
            win.commands.insert(command::gained_focus);
            break;

        case APP_CMD_LOST_FOCUS:
            win.commands.insert(command::lost_focus);
            break;

        default:
            break;
    }
}

}  // namespace

context::context() noexcept
{
    assert(asset::android_activity);
    auto& egl = egl_display::cast(data);

    LOGDD("context::context");

    egl.android = asset::android_activity;
    egl.android->onAppCmd = android_handle_command;
    egl.android->onInputEvent = android_handle_input;
    egl.android->userData = this;

    egl.display = EGL_NO_DISPLAY;
    egl.context = EGL_NO_CONTEXT;
    egl.surface = EGL_NO_SURFACE;
}

void context::buffer_swap() noexcept
{
    auto& egl = egl_display::cast(data);
    eglSwapBuffers(egl.display, egl.surface);
}

void context::event_loop_back(bool block_if_possible) noexcept
{
    int events;
    android_poll_source * source;
    auto& egl = egl_display::cast(data);

    while (!commands.is_full() &&
            (/*ident =*/ ALooper_pollAll(block_if_possible ? -1 : 0, nullptr, &events, reinterpret_cast<void**>(&source))) >= 0)
    {
        // Process this event.
        if (!!source)
            source->process(egl.android, source);

        // Check if we are exiting.
        if (!!egl.android->destroyRequested)
        {
            LOGW("Activity destruction requested.");
            commands.insert(command::close_window);
        }

        block_if_possible = false;
    }
}

context::~context() noexcept
{
    LOGDD("context::~context");
    terminate_display();
}

bool context::has_opengl() const noexcept
{
    auto& egl = egl_display::cast(data);
    return egl.display != EGL_NO_DISPLAY;
}

void context::terminate_display() noexcept
{
    auto& egl = egl_display::cast(data);

    LOGD("context::terminate_display");

    if (egl.display != EGL_NO_DISPLAY) {
        eglMakeCurrent(egl.display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (egl.context != EGL_NO_CONTEXT)
            eglDestroyContext(egl.display, egl.context);
        if (egl.surface != EGL_NO_SURFACE)
            eglDestroySurface(egl.display, egl.surface);
        eglTerminate(egl.display);
    }
    egl.display = EGL_NO_DISPLAY;
    egl.context = EGL_NO_CONTEXT;
    egl.surface = EGL_NO_SURFACE;
}

struct android_app * asset::android_activity = nullptr;

asset::asset(AAsset* f, std::string_view d) noexcept
    : file(f), data(d)
{}

asset::asset(asset&& other) noexcept
    : file(other.file), data(other.data)
{
    other.file = nullptr;
}

asset::~asset() noexcept
{
    if (file)
        AAsset_close(file);
}

asset::operator bool() const noexcept
{
    return !!data.size();
}

std::string_view asset::view() const noexcept
{
    return data;
}

asset asset::hold(std::string path) noexcept
{
    return hold(path.c_str());
}

asset asset::hold(const char * path) noexcept
{
    std::unique_ptr<AAsset, decltype(&AAsset_close)> file{
            AAssetManager_open(android_activity->activity->assetManager, path, AASSET_MODE_BUFFER),
            AAsset_close
        };

    if (auto data = file ? AAsset_getBuffer(file.get()) : nullptr; !data)
    {
        LOGE("Error loading asset `%s`", path);
        return {};
    }
    else
    {
        const std::string_view view{ reinterpret_cast<const char*>(data), static_cast<size_t>(AAsset_getLength(file.get())) };
        return { file.release(), view };
    }
}

}  // namespace platform
