#ifndef MAIN_WASM_H__
#define MAIN_WASM_H__

// Emscripten/WebAssembly entry point for Minecraft PE Alpha 0.6.1.
//
// Key differences from main_linux.h:
//  • GL context: OpenGL ES 2.0 (WebGL 1) — fixed-function calls are translated
//    by Emscripten's LEGACY_GL_EMULATION layer (link with -s LEGACY_GL_EMULATION=1).
//  • Main loop must use emscripten_set_main_loop_arg(); a blocking while() loop
//    prevents the browser from processing events and will hang the tab.
//  • No signal handling — the browser runtime handles SIGINT/SIGTERM.
//  • File system: Emscripten MemFS; assets embedded via --preload-file at /data/.

#include <SDL2/SDL.h>
#include <emscripten.h>
#include <emscripten/html5.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include "App.h"
#include "AppPlatform_wasm.h"
#include "platform/input/Keyboard.h"
#include "platform/input/Mouse.h"
#include "platform/input/Multitouch.h"
#include "platform/log.h"

// ── Global state ───────────────────────────────────────────────────────────
// Required because emscripten_set_main_loop_arg uses a C callback.

struct WasmState {
    MAIN_CLASS*   app;
    SDL_Window*   window;
    SDL_GLContext gl_ctx;
};

static WasmState g_wasm_state;

// ── Key mapping ────────────────────────────────────────────────────────────
// SDL keycodes are platform-independent; mapping is identical to Linux.

static unsigned char wasm_transform_key(SDL_Keycode sym) {
    if (sym >= SDLK_a && sym <= SDLK_z)
        return static_cast<unsigned char>(sym - 32);
    if (sym >= SDLK_0 && sym <= SDLK_9)
        return static_cast<unsigned char>(sym);

    if (sym == SDLK_BACKSPACE) return 8;
    if (sym == SDLK_TAB)       return 9;
    if (sym == SDLK_RETURN)    return 13;
    if (sym == SDLK_ESCAPE)    return Keyboard::KEY_ESCAPE;
    if (sym == SDLK_SPACE)     return Keyboard::KEY_SPACE;

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
        case SDLK_RCTRL:    return 17;
        case SDLK_DELETE:   return 46;
        case SDLK_INSERT:   return 45;
        case SDLK_HOME:     return 36;
        case SDLK_END:      return 35;
        case SDLK_PAGEUP:   return 33;
        case SDLK_PAGEDOWN: return 34;
        default:            return 0;
    }
}

// ── Event dispatch ─────────────────────────────────────────────────────────

static void wasm_handle_events(App* app) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {

        case SDL_QUIT:
            app->quit();
            break;

        case SDL_WINDOWEVENT:
            if (event.window.event == SDL_WINDOWEVENT_RESIZED)
                app->setSize(event.window.data1, event.window.data2);
            break;

        case SDL_KEYDOWN: {
            unsigned char key = wasm_transform_key(event.key.keysym.sym);
            if (key) Keyboard::feed(key, KeyboardAction::KEYDOWN);
            break;
        }
        case SDL_KEYUP: {
            unsigned char key = wasm_transform_key(event.key.keysym.sym);
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
            Mouse::feed(0, 0, static_cast<short>(x), static_cast<short>(y),
                        event.motion.xrel, event.motion.yrel);
            Multitouch::feed(0, 0,
                             static_cast<short>(x), static_cast<short>(y), 0);
            break;
        }
        case SDL_MOUSEBUTTONDOWN: {
            bool left   = (event.button.button == SDL_BUTTON_LEFT);
            char button = left ? 1 : 2;
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
            short dy = (event.wheel.y > 0) ? 1 : -1;
            Mouse::feed(3, 0,
                        static_cast<short>(mx), static_cast<short>(my),
                        0, dy);
            break;
        }

        // Touch events — forward to Multitouch (browser canvas touch).
        case SDL_FINGERDOWN:
        case SDL_FINGERMOTION:
        case SDL_FINGERUP: {
            short tx = static_cast<short>(event.tfinger.x  * 854);
            short ty = static_cast<short>(event.tfinger.y  * 480);
            char  action = (event.type == SDL_FINGERDOWN)   ? 1
                         : (event.type == SDL_FINGERUP)     ? 0
                                                            : 0; // MOVE
            Multitouch::feed(1, action, tx, ty, 0);
            break;
        }

        default:
            break;
        }
    }
}

// ── Per-frame callback ─────────────────────────────────────────────────────

static void wasm_main_loop(void* arg) {
    WasmState* s = static_cast<WasmState*>(arg);
    wasm_handle_events(s->app);
    s->app->update();
    SDL_GL_SwapWindow(s->window);
}

// ── main ───────────────────────────────────────────────────────────────────

int main(int /*argc*/, char** /*argv*/) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    // WebGL context via SDL2 / Emscripten.
    // LEGACY_GL_EMULATION (linker flag) translates GLES 1.x fixed-function
    // calls (glBegin/glEnd, glMatrixMode, etc.) into WebGL 1 shader calls.
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);

    const int kWidth  = 854;
    const int kHeight = 480;

    SDL_Window* window = SDL_CreateWindow(
        "Minecraft PE Alpha 0.6.1",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        kWidth, kHeight,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

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

    // ── Platform setup ─────────────────────────────────────────────────────
    static AppPlatform_wasm platform;
    platform.SetScreenDims(kWidth, kHeight);

    AppContext context;
    context.platform = &platform;
    context.doRender = true;

    // ── App initialisation ─────────────────────────────────────────────────
    g_wasm_state.app    = new MAIN_CLASS();
    g_wasm_state.window = window;
    g_wasm_state.gl_ctx = gl_ctx;

    MAIN_CLASS* app = g_wasm_state.app;
    app->externalStoragePath      = "/minecraft-pe";
    app->externalCacheStoragePath = "/minecraft-pe/cache";

    app->App::init(context);
    app->setSize(kWidth, kHeight);

    LOGI("Minecraft PE Alpha 0.6.1 — WebAssembly/Emscripten\n");

    // ── Main loop ──────────────────────────────────────────────────────────
    // emscripten_set_main_loop_arg(callback, arg, fps, simulate_infinite_loop)
    //   fps = 0  → use requestAnimationFrame (capped to display refresh rate)
    //   simulate_infinite_loop = 1 → this call never returns; the C stack is
    //   unwound internally, so all cleanup must happen in the callback or an
    //   atexit handler.
    emscripten_set_main_loop_arg(wasm_main_loop, &g_wasm_state, 0, 1);

    // Unreachable — Emscripten throws a JS exception to unwind the stack.
    return 0;
}

#endif  // MAIN_WASM_H__
