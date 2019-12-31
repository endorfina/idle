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

#include<cstdio>
#include<memory>
#include<X11/X.h>
#include<X11/Xlib.h>
#include<X11/Xutil.h>

#include "opengl_core_adaptive.hpp"
#include <GL/glx.h>
#include <cstdlib>

#include "display.hpp"
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

    static const x11_display& cast(const window::data_t& data)
    {
        return *reinterpret_cast<const x11_display*>(data);
    }

    static x11_display& cast(window::data_t& data)
    {
        return *reinterpret_cast<x11_display*>(data);
    }
};

//Atom                    wmDeleteMessage;

int x_fatal_error_handler(Display *) {
    std::fputs(u8"\U0001F480 (Display destroyed)\n", stderr);
    std::exit(0x0);
    return 0;
}

int x_error_handler(Display *dpy, XErrorEvent *ev) {
    char str[250];
    XGetErrorText(dpy, ev->error_code, str, 250);
    std::fprintf(stderr, "%s\n", str);
    std::terminate();
    return 0;
}

static_assert(sizeof(x11_display) <= sizeof(window::data_t));

} // namespace

window::window()
{
    auto& x = x11_display::cast(data);

    LOGD("window::window");

    x.display = nullptr;

    XSetWindowAttributes setwindowattributes;

    GLint attributes[] = {
        GLX_X_RENDERABLE    , True,
        GLX_DRAWABLE_TYPE   , GLX_WINDOW_BIT,
        GLX_RENDER_TYPE     , GLX_RGBA_BIT,
        GLX_X_VISUAL_TYPE   , GLX_TRUE_COLOR,
        GLX_RED_SIZE        , 4,
        GLX_GREEN_SIZE      , 4,
        GLX_BLUE_SIZE       , 4,
        GLX_ALPHA_SIZE      , 4,
        GLX_DEPTH_SIZE      , 16,
        None
    };

    std::unique_ptr<Display, decltype(&XCloseDisplay)> display(XOpenDisplay(nullptr), XCloseDisplay);

    if(!display) {
        fputs("Cannot connect to X server\n", stderr);
        std::abort();
    }

    if (int glx_major, glx_minor;
        !glXQueryVersion(display.get(), &glx_major, &glx_minor)
        || ((glx_major == 1) && (glx_minor < 3)) || (glx_major < 1))
    {
        fputs("Invalid GLX version\n", stderr);
        std::abort();
    }

    GLXFBConfig bestFbc; // Get framebuffers
    int fbcount;
    if (GLXFBConfig* fbc = glXChooseFBConfig(display.get(), DefaultScreen(display.get()), attributes, &fbcount)) {
        int best_fbc = -1, worst_fbc = -1, best_num_samp = -1, worst_num_samp = 999;
        for (int i = 0; i < fbcount; ++i)
            if (std::unique_ptr<XVisualInfo, decltype(&XFree)> vi(glXGetVisualFromFBConfig(display.get(), fbc[i]), XFree); bool(vi))
            {
                int samp_buf, samples;
                glXGetFBConfigAttrib(display.get(), fbc[i], GLX_SAMPLE_BUFFERS, &samp_buf );
                glXGetFBConfigAttrib(display.get(), fbc[i], GLX_SAMPLES       , &samples  );

                if ( best_fbc < 0 || (samp_buf && samples > best_num_samp))
                    best_fbc = i, best_num_samp = samples;
                if ( worst_fbc < 0 || (!samp_buf || samples < worst_num_samp))
                    worst_fbc = i, worst_num_samp = samples;
            }
        bestFbc = fbc[best_fbc];
        XFree(fbc);
    }
    else {
        fputs("Failed to retrieve a framebuffer config\n", stderr);
        std::abort();
    }

    std::unique_ptr<XVisualInfo, decltype(&XFree)> visualinfo(glXGetVisualFromFBConfig(display.get(), bestFbc), XFree);

    if(!visualinfo) {
        fputs("No appropriate visual found\n", stderr);
        std::abort();
    }

    x.colormap = XCreateColormap(display.get(), RootWindow(display.get(), visualinfo->screen), visualinfo->visual, AllocNone);

    setwindowattributes.colormap = x.colormap;
    setwindowattributes.event_mask = ButtonPressMask | StructureNotifyMask | ButtonReleaseMask |
                          //EnterWindowMask | LeaveWindowMask |
                          KeyPressMask | // KeyReleaseMask |
                          PointerMotionMask | Button1MotionMask; // | VisibilityChangeMask | ExposureMask;

    x.window = XCreateWindow(display.get(), RootWindow(display.get(), visualinfo->screen), 0, 0, 960, 720, 0, visualinfo->depth, InputOutput, visualinfo->visual, CWColormap | CWEventMask, &setwindowattributes);

    if(!x.window) {
        fputs("Failed to create a window\n", stderr);
        XFreeColormap(display.get(), x.colormap);
        std::abort();
    }
    XStoreName(display.get(), x.window, "idle/crimson");
    XMapWindow(display.get(), x.window);

    //auto old_fatal_handler =
        XSetIOErrorHandler(x_fatal_error_handler);
    auto old_handler =
        XSetErrorHandler(x_error_handler);
    x.context = glXCreateContext(display.get(), visualinfo.get(), NULL, True);

    if (!x.context) {
        XDestroyWindow(display.get(), x.window);
        XFreeColormap(display.get(), x.colormap);
        fputs("Failed to create a proper gl context\n", stderr);
        std::abort();
    }

    // wmDeleteMessage = XInternAtom(display.get(), "WM_DELETE_WINDOW", False);
    // XSetWMProtocols(display.get(), window, &wmDeleteMessage, 1);

    // Sync to ensure any errors generated are processed.
    XSync(display.get(), False);
    //XSetIOErrorHandler(old_fatal_handler);
    XSetErrorHandler(old_handler);

    if (!glXIsDirect(display.get(), x.context)) {
        fputs(u8"Indirect GLX rendering context obtained (\U0001f937 why\?\?)\n", stderr);
    }

    glXMakeCurrent(display.get(), x.window, x.context);


    if(const static gl::exts::LoadTest glTest = gl::sys::LoadFunctions(); !glTest)
    {
        fputs("Failed to load crucial OpenGL functions\n", stderr);
        glXMakeCurrent(display.get(), None, NULL);
        glXDestroyContext(display.get(), x.context);
        XDestroyWindow(display.get(), x.window);
        XFreeColormap(display.get(), x.colormap);
        std::abort();
    }
    else if (const auto amt = glTest.GetNumMissing(); amt > 0)
        fprintf(stderr, "Number of functions that failed to load: %i.\n", amt);

    gl::ClearColor(background.r, background.g, background.b, 1);
    gl::Clear(gl::COLOR_BUFFER_BIT);
    glXSwapBuffers(display.get(), x.window);

    x.display = display.release();

    resize_request.emplace(resize_request_t{960, 720, -1, -1, std::chrono::system_clock::now()});
    commands.push_back(command::InitWindow);
}

void window::buffer_swap()
{
    auto& x = x11_display::cast(data);
    glXSwapBuffers(x.display, x.window);
}

void window::event_loop_back(bool block_if_possible)
{
    auto& x = x11_display::cast(data);
    XEvent xev;
    {
        // Otherwise the mouse won't work.
        Window root_window;
        int root_x, root_y; //<--two
        unsigned int mask; //<--three
        XQueryPointer(x.display, x.window, &root_window, &root_window, &root_x, &root_y, &root_x, &root_y, &mask);
    }
    while(!!XQLength(x.display))
    {
        XNextEvent(x.display, &xev);
        switch (xev.type) {
            case ButtonPress:
            case ButtonRelease:
                cursor.pos.x = static_cast<float>(xev.xbutton.x);
                cursor.pos.y = static_cast<float>(xev.xbutton.y);
                cursor.pressed = xev.xbutton.type == ButtonPress;
                break;
            case EnterNotify:
            case MotionNotify:
            case LeaveNotify:
                cursor.pos.x = static_cast<float>(xev.xmotion.x);
                cursor.pos.y = static_cast<float>(xev.xmotion.y);
                break;
            case ConfigureNotify:
                glXMakeCurrent(x.display, x.window, x.context);
                resize_request.emplace(resize_request_t{xev.xconfigure.width, xev.xconfigure.height, -1, -1, std::chrono::system_clock::now() + std::chrono::seconds(1)});
                break;
            case KeyPress:
                if (xev.xkey.keycode == 0x9)
                {
                    commands.push_back(command::PausePressed);
                }
                break;
            default:
                break;
        }
    }
}

window::~window()
{
    LOGD("window::~window");
    terminate_display();
}

bool window::has_gl_context() const
{
    auto& x = x11_display::cast(data);
    return !!x.display;
}

void window::terminate_display()
{
    auto& x = x11_display::cast(data);

    LOGD("window::terminate_display");

    if (x.display) {
        glXMakeCurrent(x.display, None, NULL);
        glXDestroyContext(x.display, x.context);
        XDestroyWindow(x.display, x.window);
        XFreeColormap(x.display, x.colormap);
        XCloseDisplay(x.display);
        x.display = nullptr;
    }
}

asset::~asset() {}

asset::operator bool() const
{
    return !!size;
}

std::string_view asset::view() const
{
    return { ptr.get(), size };
}

asset asset::hold(std::string path)
{
    path.insert(0, "assets/");

    if(const std::unique_ptr<FILE, decltype(&fclose)> f{ fopen(path.c_str(), "rb"), fclose })
    {
        const auto size = ftell(f.get());
        if(size <= 0 || fseek(f.get(), 0, SEEK_SET) != 0)
            return {};

        auto ptr = std::make_unique<char[]>(size);

        if(fread(ptr.get(), 1, size, f.get()) == static_cast<size_t>(size))
            return { static_cast<size_t>(size), std::move(ptr) };
    }
    return {};
}

asset asset::hold(const char * path)
{
    return hold(path);
}

}  // namespace platform
