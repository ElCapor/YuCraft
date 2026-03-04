#ifndef MAIN_LINUX_H__
#define MAIN_LINUX_H__

// SDL2 entry point for Linux desktop (X11 and Wayland via SDL_VIDEODRIVER).
// This file is included by main.cpp when -DLINUX is defined.
// SDL2 selects the Wayland backend automatically when:
//   SDL_VIDEODRIVER=wayland ./minecraft-pe
// or falls back to X11 when Wayland is unavailable.

#include <SDL2/SDL.h>
#include <epoxy/gl.h>

#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include "App.h"
#include "AppPlatform_linux.h"
#include "platform/input/Keyboard.h"
#include "platform/input/Mouse.h"
#include "platform/input/Multitouch.h"
#include "platform/log.h"

static volatile bool g_linux_quit = false;

static void linux_signal_handler(int /*signum*/) {
    g_linux_quit = true;
}

// ── Key mapping ────────────────────────────────────────────────────────────
// Map an SDL2 Keycode to the game's internal key constant (Windows VK codes).
// Returns 0 if the key should be ignored.
//
// The game's Keyboard class uses:
//   - Uppercase ASCII (65-90) for letter keys
//   - ASCII digits (48-57) for number keys
//   - ASCII control chars for Backspace(8), Tab(9), Return(13), Escape(27)
//   - Windows virtual-key codes for function/navigation keys

static unsigned char linux_transform_key(SDL_Keycode sym) {
    // Letter keys: SDL gives lowercase ASCII; game expects uppercase ASCII.
    if (sym >= SDLK_a && sym <= SDLK_z)
        return static_cast<unsigned char>(sym - 32);

    // Digit and punctuation keys that are ASCII-compatible.
    if (sym >= SDLK_0 && sym <= SDLK_9)
        return static_cast<unsigned char>(sym);

    // ASCII control / printable characters that map 1:1.
    if (sym == SDLK_BACKSPACE) return 8;
    if (sym == SDLK_TAB)       return 9;
    if (sym == SDLK_RETURN)    return 13;
    if (sym == SDLK_ESCAPE)    return Keyboard::KEY_ESCAPE;
    if (sym == SDLK_SPACE)     return Keyboard::KEY_SPACE;

    // Extended keys — map to Windows VK codes used by the game.
    switch (sym) {
        case SDLK_F1:       return 112;
        case SDLK_F2:       return 113;
        case SDLK_F3:       return 114;
        case SDLK_F4:       return 115;
        case SDLK_F5:       return 116;
        case SDLK_F6:       return 117;
        case SDLK_F7:       return 118;
        case SDLK_F8:       return 119;
        case SDLK_F9:       return 120;
        case SDLK_F10:      return 121;
        case SDLK_F11:      return 122;
        case SDLK_F12:      return 123;
        case SDLK_LEFT:     return 37;
        case SDLK_UP:       return 38;
        case SDLK_RIGHT:    return 39;
        case SDLK_DOWN:     return 40;
        case SDLK_LSHIFT:
        case SDLK_RSHIFT:   return Keyboard::KEY_LSHIFT;
        case SDLK_LCTRL:
        case SDLK_RCTRL:    return 17;   // VK_CONTROL
        case SDLK_DELETE:   return 46;   // VK_DELETE
        case SDLK_INSERT:   return 45;   // VK_INSERT
        case SDLK_HOME:     return 36;   // VK_HOME
        case SDLK_END:      return 35;   // VK_END
        case SDLK_PAGEUP:   return 33;   // VK_PRIOR
        case SDLK_PAGEDOWN: return 34;   // VK_NEXT
        default:            return 0;
    }
}

// ── Event dispatch ─────────────────────────────────────────────────────────

static void linux_handle_events(App* app) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {

        case SDL_QUIT:
            app->quit();
            break;

        case SDL_WINDOWEVENT:
            if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                app->setSize(event.window.data1, event.window.data2);
            }
            break;

        case SDL_KEYDOWN: {
            unsigned char key = linux_transform_key(event.key.keysym.sym);
            if (key) Keyboard::feed(key, KeyboardAction::KEYDOWN);
            break;
        }
        case SDL_KEYUP: {
            unsigned char key = linux_transform_key(event.key.keysym.sym);
            if (key) Keyboard::feed(key, KeyboardAction::KEYUP);
            break;
        }
        case SDL_TEXTINPUT:
            for (const char* c = event.text.text; *c != '\0'; ++c)
                Keyboard::feedText(*c);
            break;

        case SDL_MOUSEMOTION: {
            float x = static_cast<float>(event.motion.x);
            float y = static_cast<float>(event.motion.y);
            // ACTION_MOVE=0, DATA=0
            Mouse::feed(0, 0, static_cast<short>(x), static_cast<short>(y),
                        event.motion.xrel, event.motion.yrel);
            Multitouch::feed(0, 0,
                             static_cast<short>(x), static_cast<short>(y), 0);
            break;
        }

        case SDL_MOUSEBUTTONDOWN: {
            bool left   = (event.button.button == SDL_BUTTON_LEFT);
            char button = left ? 1 : 2;  // ACTION_LEFT=1, ACTION_RIGHT=2
            Mouse::feed(button, 1, event.button.x, event.button.y);
            Multitouch::feed(button, 1, event.button.x, event.button.y, 0);
            break;
        }
        case SDL_MOUSEBUTTONUP: {
            bool left   = (event.button.button == SDL_BUTTON_LEFT);
            char button = left ? 1 : 2;
            Mouse::feed(button, 0, event.button.x, event.button.y);
            Multitouch::feed(button, 0, event.button.x, event.button.y, 0);
            break;
        }

        case SDL_MOUSEWHEEL: {
            int mx = 0, my = 0;
            SDL_GetMouseState(&mx, &my);
            // ACTION_WHEEL=3; dx=0, dy=±1 for wheel direction
            short dy = (event.wheel.y > 0) ? 1 : -1;
            Mouse::feed(3, 0,
                        static_cast<short>(mx), static_cast<short>(my),
                        0, dy);
            break;
        }

        default:
            break;
        }
    }
}

// ── main ───────────────────────────────────────────────────────────────────

int main(int /*argc*/, char** /*argv*/) {
    std::signal(SIGINT,  linux_signal_handler);
    std::signal(SIGTERM, linux_signal_handler);

    // ── SDL2 initialisation ────────────────────────────────────────────────
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    // Legacy compatibility profile: the game uses fixed-function GL 1.x.
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,   16);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE,      8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,    8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,     8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,    8);

    const int kWidth  = 1280;
    const int kHeight = 720;

    SDL_Window* window = SDL_CreateWindow(
        "Minecraft PE Alpha 0.6.1",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        kWidth, kHeight,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

    if (!window) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_GLContext gl_ctx = SDL_GL_CreateContext(window);
    if (!gl_ctx) {
        fprintf(stderr, "SDL_GL_CreateContext failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_GL_MakeCurrent(window, gl_ctx);
    SDL_GL_SetSwapInterval(1);  // vsync

    // libepoxy auto-initialises on the first GL call; no explicit init needed.
    LOGI("OpenGL %s (epoxy — GLX/EGL transparent)\n", glGetString(GL_VERSION));

    // ── Platform setup ─────────────────────────────────────────────────────
    AppPlatform_linux platform;
    platform.SetScreenDims(kWidth, kHeight);

    const char* home_env = std::getenv("HOME");
    std::string storage_path = home_env
        ? std::string(home_env) + "/.minecraft-pe"
        : "./.minecraft-pe";

    platform.setStoragePath(storage_path + "/");

    // ── App initialisation ─────────────────────────────────────────────────
    AppContext context;
    context.platform = &platform;
    context.doRender = true;

    MAIN_CLASS* app = new MAIN_CLASS();
    // externalStoragePath is a public field of Minecraft; set before init().
    app->externalStoragePath      = storage_path;
    app->externalCacheStoragePath = storage_path + "/cache";

    app->App::init(context);  // explicit base-class call: NinecraftApp::init() hides App::init(AppContext&)
    app->setSize(kWidth, kHeight);

    LOGI("Minecraft PE Alpha 0.6.1 — Linux/SDL2 (%s)\n",
         SDL_GetCurrentVideoDriver());

    // ── Main loop ──────────────────────────────────────────────────────────
    while (!app->wantToQuit() && !g_linux_quit) {
        linux_handle_events(app);
        app->update();
        SDL_GL_SwapWindow(window);
    }

    // ── Teardown ───────────────────────────────────────────────────────────
    app->destroy();
    delete app;

    SDL_GL_DeleteContext(gl_ctx);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

#endif  // MAIN_LINUX_H__
