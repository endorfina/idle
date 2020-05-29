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

#include <cstdio>
#include <memory>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "opengl_core_adaptive.hpp"
#include <GL/glx.h>
#include <cstdlib>
#include <atomic>

#include <log.hpp>
#include "context.hpp"
#include "asset_access.hpp"

namespace platform
{
namespace
{

struct x11_display
{
    Display                 *display;
    Window                  window;
    GLXContext              context;
    Colormap                colormap;

    x11_display() = delete;

    static const x11_display& cast(const context::data_t& data)
    {
        return *std::launder(reinterpret_cast<const x11_display*>(data));
    }

    static x11_display& cast(context::data_t& data)
    {
        return *std::launder(reinterpret_cast<x11_display*>(data));
    }
};

#ifdef X11_USE_CLIENTMESSAGE
Atom wmDeleteMessage;
#endif

int x_fatal_error_handler(Display *)
{
    LOGE(u8"\U0001F480 (Display destroyed)");
    return 0;
}

int x_error_handler(Display *dpy, XErrorEvent *ev)
{
    char str[250];
    XGetErrorText(dpy, ev->error_code, str, 250);
    LOGE("%s", str);
    return 0;
}

static_assert(sizeof(x11_display) <= sizeof(context::data_t));

} // namespace

context::context()
{
    auto& x = x11_display::cast(data);

    LOGD("context::context");

    x.display = nullptr;

    XSetWindowAttributes setwindowattributes;

    GLint attributes[] {
        GLX_X_RENDERABLE  , True,
        GLX_DRAWABLE_TYPE , GLX_WINDOW_BIT,
        GLX_RENDER_TYPE   , GLX_RGBA_BIT,
        GLX_X_VISUAL_TYPE , GLX_TRUE_COLOR,
        GLX_RED_SIZE      , 4,
        GLX_GREEN_SIZE    , 4,
        GLX_BLUE_SIZE     , 4,
        GLX_ALPHA_SIZE    , 4,
        GLX_DEPTH_SIZE    , 16,
        None
    };

    std::unique_ptr<Display, decltype(&XCloseDisplay)> display{ XOpenDisplay(nullptr), XCloseDisplay };

    if(!display) {
        LOGE("Cannot connect to X server");
        commands.insert(command::CloseWindow);
        return;
    }

    if (int glx_major, glx_minor;
        !glXQueryVersion(display.get(), &glx_major, &glx_minor)
        || ((glx_major == 1) && (glx_minor < 3)) || (glx_major < 1))
    {
        LOGE("Invalid GLX version");
        commands.insert(command::CloseWindow);
        return;
    }

    GLXFBConfig bestFbc; // Get framebuffers
    int fbcount;
    if (const std::unique_ptr<GLXFBConfig[], decltype(&XFree)> fbc{
            glXChooseFBConfig(display.get(), DefaultScreen(display.get()), attributes, &fbcount),
            XFree })
    {
        int best_fbc = -1, worst_fbc = -1, best_num_samp = -1, worst_num_samp = 999;

        for (int i = 0; i < fbcount; ++i)
        {
            if (const std::unique_ptr<XVisualInfo, decltype(&XFree)> vi{
                    glXGetVisualFromFBConfig(display.get(), fbc[i]),
                    XFree })
            {
                int samp_buf, samples;
                glXGetFBConfigAttrib(display.get(), fbc[i], GLX_SAMPLE_BUFFERS, &samp_buf);
                glXGetFBConfigAttrib(display.get(), fbc[i], GLX_SAMPLES, &samples);

                if (best_fbc < 0 || (samp_buf && samples > best_num_samp))
                {
                    best_fbc = i;
                    best_num_samp = samples;
                }

                if (worst_fbc < 0 || (!samp_buf || samples < worst_num_samp))
                {
                    worst_fbc = i;
                    worst_num_samp = samples;
                }
            }
        }
        bestFbc = fbc[best_fbc];
    }
    else
    {
        LOGE("Failed to retrieve a framebuffer config");
        commands.insert(command::CloseWindow);
        return;
    }

    const std::unique_ptr<XVisualInfo, decltype(&XFree)> visualinfo{
            glXGetVisualFromFBConfig(display.get(), bestFbc),
            XFree };

    if(!visualinfo)
    {
        LOGE("No appropriate visual found");
        commands.insert(command::CloseWindow);
        return;
    }

    x.colormap = XCreateColormap(display.get(), RootWindow(display.get(), visualinfo->screen), visualinfo->visual, AllocNone);

    setwindowattributes.colormap = x.colormap;
    setwindowattributes.event_mask = ButtonPressMask | StructureNotifyMask | ButtonReleaseMask |
                          //EnterWindowMask | LeaveWindowMask |
                          KeyPressMask | // KeyReleaseMask |
                          PointerMotionMask | Button1MotionMask; // | VisibilityChangeMask | ExposureMask;

    constexpr unsigned initial_width = 960, initial_height = 720;

    x.window = XCreateWindow(display.get(), RootWindow(display.get(), visualinfo->screen), 0, 0, initial_width, initial_height, 0, visualinfo->depth, InputOutput, visualinfo->visual, CWColormap | CWEventMask, &setwindowattributes);

    if(!x.window)
    {
        LOGE("Failed to create a window");
        XFreeColormap(display.get(), x.colormap);
        commands.insert(command::CloseWindow);
        return;
    }
    XStoreName(display.get(), x.window, "idle/crimson");
    XMapWindow(display.get(), x.window);


    auto default_error_handler = XSetErrorHandler(x_error_handler);
    XSetIOErrorHandler(x_fatal_error_handler);

    x.context = glXCreateContext(display.get(), visualinfo.get(), NULL, True);

    if (!x.context)
    {
        XDestroyWindow(display.get(), x.window);
        XFreeColormap(display.get(), x.colormap);
        LOGE("Failed to create a proper gl context");
        commands.insert(command::CloseWindow);
        return;
    }

#ifdef X11_USE_CLIENTMESSAGE
    wmDeleteMessage = XInternAtom(display.get(), "WM_DELETE_WINDOW", False);
    XSetWMProtocols(display.get(), x.window, &wmDeleteMessage, 1);
#endif

    // Sync to ensure any errors generated are processed.
    XSync(display.get(), False);
    XSetErrorHandler(default_error_handler);

    if (!glXIsDirect(display.get(), x.context))
    {
        LOGE(u8"Indirect GLX rendering context obtained (\U0001f937 why\?\?)");
    }

    glXMakeCurrent(display.get(), x.window, x.context);


    if (const static gl::exts::LoadTest glTest = gl::sys::LoadFunctions(); !glTest)
    {
        LOGE("Failed to load crucial OpenGL functions");
        glXMakeCurrent(display.get(), None, NULL);
        glXDestroyContext(display.get(), x.context);
        XDestroyWindow(display.get(), x.window);
        XFreeColormap(display.get(), x.colormap);
        commands.insert(command::CloseWindow);
        return;
    }
    else if (const auto amt = glTest.GetNumMissing(); amt > 0)
    {
        LOGE("Number of functions that failed to load: %i.", amt);
    }

    gl::ClearColor(background.r, background.g, background.b, 1);
    gl::Clear(gl::COLOR_BUFFER_BIT);
    glXSwapBuffers(display.get(), x.window);

    x.display = display.release();

    resize_request.emplace(initial_width, initial_height);
    commands.insert(command::InitWindow);
    commands.insert(command::GainedFocus); // TODO: Make X handle focus as to reduce resource usage when idle
}

void context::buffer_swap()
{
    auto& x = x11_display::cast(data);
    glXSwapBuffers(x.display, x.window);
}

void context::event_loop_back(bool)
{
    auto& x = x11_display::cast(data);
    XEvent xev;
    {
        // Without this query call the mouse might not work at all
        Window root_window;
        int root_x, root_y;
        unsigned int mask;
        XQueryPointer(x.display, x.window, &root_window, &root_window, &root_x, &root_y, &root_x, &root_y, &mask);
    }

    while(!commands.is_full() && XQLength(x.display))
    {
        XNextEvent(x.display, &xev);
        switch (xev.type) {
            case ButtonPress:
            case ButtonRelease:
                cursor.pos = { static_cast<float>(xev.xbutton.x), static_cast<float>(xev.xbutton.y) };
                cursor.pressed = xev.xbutton.type == ButtonPress;
                break;

            case EnterNotify:
            case MotionNotify:
            case LeaveNotify:
                cursor.pos = { static_cast<float>(xev.xmotion.x), static_cast<float>(xev.xmotion.y) };
                break;

            case ConfigureNotify:
                glXMakeCurrent(x.display, x.window, x.context);
                resize_request.emplace(xev.xconfigure.width, xev.xconfigure.height);
                break;

            case KeyPress:
                if (xev.xkey.keycode == 0x9)
                {
                    commands.insert(command::PausePressed);
                }
                break;

#ifdef X11_USE_CLIENTMESSAGE
            case ClientMessage:
                commands.insert(command::CloseWindow);
                return;
#endif

            default:
                break;
        }
    }
}

context::~context()
{
    LOGD("context::~context");
    terminate_display();
}

bool context::has_opengl() const
{
    auto& x = x11_display::cast(data);
    return !!x.display;
}

void context::terminate_display()
{
    auto& x = x11_display::cast(data);

    LOGD("context::terminate_display");

    if (x.display)
    {
        glXMakeCurrent(x.display, None, NULL);
        glXDestroyContext(x.display, x.context);
        XDestroyWindow(x.display, x.window);
        XFreeColormap(x.display, x.colormap);
        XCloseDisplay(x.display);
        x.display = nullptr;
    }
}

asset::operator bool() const
{
    return !!size;
}

std::string_view asset::view() const
{
    return { reinterpret_cast<const char*>(ptr.get()), size };
}

asset asset::hold(std::string path)
{
    path.insert(0, "assets/");

    if (const std::unique_ptr<FILE, decltype(&std::fclose)> f{
            std::fopen(path.c_str(), "rb"),
            std::fclose })
    {
        std::fseek(f.get(), 0, SEEK_END);

        const auto size = std::ftell(f.get());
        if (size <= 0 || std::fseek(f.get(), 0, SEEK_SET) != 0)
            return {};

        LOGD("\"%s\" - loaded %ld %s",
                path.c_str(),
                size >= 1024 ? size / 1024 : size,
                size >= 1024 ? "KB" : "bytes");

        std::unique_ptr<unsigned char[]> ptr(new unsigned char[size]);

        if (std::fread(ptr.get(), 1, size, f.get()) == static_cast<size_t>(size))
        {
            return { std::move(ptr), static_cast<size_t>(size) };
        }
    }

    LOGE("Couldn't get a hold of \"%s\"", path.c_str());
    return {};
}

asset asset::hold(const char * path)
{
    return hold(std::string{path});
}

}  // namespace platform
