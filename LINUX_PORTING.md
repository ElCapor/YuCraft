# Linux Porting Notes — Minecraft PE Alpha 0.6.1

This document details every change made to port the codebase to Linux desktop
(X11 and Wayland), explains the root cause of each problem encountered, and
describes the solution chosen.

---

## Table of Contents

1. [Environment](#1-environment)
2. [Build System](#2-build-system)
3. [OpenGL Layer: GLEW → libepoxy](#3-opengl-layer-glew--libepoxy)
4. [Vendored libpng Conflict](#4-vendored-libpng-conflict)
5. [AppPlatform_linux](#5-appplatform_linux)
6. [Entry Point: main_linux.h](#6-entry-point-main_linuxh)
7. [World Name Input (SDL Text Input)](#7-world-name-input-sdl-text-input)
8. [FPS Mouse Lock](#8-fps-mouse-lock)
9. [AZERTY / ZQSD Layout](#9-azerty--zqsd-layout)
10. [Compiler Fixes](#10-compiler-fixes)
11. [File Inventory](#11-file-inventory)

---

## 1. Environment

| Item | Value |
|------|-------|
| OS | Arch Linux x86_64 |
| Compiler | GCC 15.2.1, `-std=c++17` |
| CMake | ≥ 3.20 |
| OpenGL | Mesa (GLX for X11, EGL for Wayland) |
| SDL2 | sdl2-compat 2.32.64 |
| libepoxy | 1.5.10 |
| libpng | 1.6.55 (system) |

The game targets OpenGL 1.x fixed-function pipeline. On Wayland the display
server exposes an EGL surface, not an XDisplay. On X11 both GLX and EGL are
available.

---

## 2. Build System

### New files

| File | Purpose |
|------|---------|
| `CMakeLists.txt` | Root CMake entry; includes platform sub-project |
| `CMakePresets.json` | `linux-release` and `linux-debug` preset definitions |
| `cmake/CompilerFlags.cmake` | Common warning/optimisation flags |
| `cmake/FindDependencies.cmake` | `find_package` / `pkg_check_modules` for all deps |
| `handheld/project/linux/CMakeLists.txt` | Game target (sources, RakNet static lib, link flags) |
| `scripts/setup_deps_linux.py` | Installs all required packages via pacman/apt |

### Key decisions

**RakNet is built inline** as a CMake static library target rather than as a
separate library or prebuilt. It shares the same build tree and its sources are
collected with `file(GLOB …)`. Two extra flags are applied only to RakNet:

```cmake
target_compile_options(raknet PRIVATE -w -fpermissive)
```

`-w` suppresses the large volume of legacy warnings. `-fpermissive` is required
because RakNet uses braced-init-list narrowing (`0xFF` into `char`) which
became a hard error in GCC's C++11 mode (see §10).

**`handheld/lib/include` is excluded from the include path.** That directory
contains a vendored libpng 1.2.37 header. Including it breaks texture loading
(see §4).

**Build command:**

```sh
cmake --preset linux-release
cmake --build build/linux-release --target minecraft-pe
```

**Run:**

```sh
cd build/linux-release/handheld/project/linux
./minecraft-pe                        # X11
SDL_VIDEODRIVER=wayland ./minecraft-pe  # force Wayland
```

---

## 3. OpenGL Layer: GLEW → libepoxy

### Problem

The existing code called `glewInit()` at startup. Under Wayland, GLEW needs an
`XDisplay` pointer to load function pointers via GLX. There is no XDisplay on a
native Wayland session, so `glewInit()` returned `GLEW_ERROR_NO_GLX_DISPLAY`
and no GL extension functions were loaded.

### Solution

Replace GLEW with **libepoxy**. libepoxy is a transparent GL dispatch library
that introspects the current context at call time and loads function pointers
via the correct mechanism (GLX on X11, EGL on Wayland). No explicit
initialisation call is required.

### Changes

**`handheld/src/client/renderer/gles.h`** — desktop GL branch:
```cpp
// Before:
#include <GL/glew.h>
#include <GL/gl.h>
// After:
#include <epoxy/gl.h>
```

**`handheld/src/client/renderer/gl.h`** — same substitution.

**`handheld/src/client/renderer/gl.cpp`** — `glInit()`:
```cpp
void glInit() {
#ifndef LINUX
    GLenum err = glewInit();  // skipped on Linux; epoxy needs no init
    printf("GLEW Init Error: %d\n", err);
#endif
}
```

**`handheld/project/linux/CMakeLists.txt`** — dependency:
```cmake
# Before:
target_link_libraries(… GLEW::GLEW …)
# After:
target_link_libraries(… PkgConfig::epoxy …)
```

---

## 4. Vendored libpng Conflict

### Problem

After GL was working, all textures rendered as white squares. The log showed:

```
libpng warning: Application built with libpng-1.2.37 but running with 1.6.55
```

The include path contained `handheld/lib/include/` which provides the vendored
`png.h` from libpng **1.2.37**. The game was compiled against the 1.2.37 API
(with its `png_struct` layout), but at runtime Debian/Arch ships libpng
**1.6.55**. The two versions have incompatible `png_struct` ABI: function calls
into the library operated on misaligned struct fields, corrupting every row
read and producing a solid white result.

### Solution

Remove `handheld/lib/include` from `target_include_directories`. System headers
arrive correctly through the CMake-imported `PNG::PNG` and `ZLIB::ZLIB` targets.

```cmake
# Removed from target_include_directories:
# "${CMAKE_SOURCE_DIR}/handheld/lib/include"
```

---

## 5. AppPlatform_linux

### Design

`AppPlatform_linux` is a new class inheriting from `AppPlatform`. It provides
the Linux implementations of all platform services the game queries.

**`handheld/src/AppPlatform_linux.h`**:

```
class AppPlatform_linux : public AppPlatform {
 public:
  void SetScreenDims(int width, int height);
  int  getScreenWidth()  override;
  int  getScreenHeight() override;
  void setStoragePath(const std::string& path);

  // Text input dialog (non-blocking, for world name entry)
  void createUserInput(const std::string& hint, …) override;
  int  getUserInputStatus() override;      // -1=waiting, 0=cancelled, 1=confirmed
  std::string getUserInput() override;

  void onTextInput(const char* text);     // SDL_TEXTINPUT event
  void onTextConfirm();                   // Enter key
  void onTextCancel();                    // Escape key
  void onTextBackspace();

  bool isInTextInputMode() const;

  // AppPlatform overrides
  ImageBuffer loadTexture(const std::string& path, bool isPng) override;
  std::string readAssetFile(const std::string& path) override;
  std::string getStoragePath(const std::string& filename) override;

 private:
  int screen_width_{1280};
  int screen_height_{720};
  std::string asset_base_path_{"./data/"};
  std::string storage_path_;
  // text input state
  bool text_input_active_{false};
  int  text_input_status_{-1};
  std::string text_input_buffer_;
};
```

**`handheld/src/AppPlatform_linux.cpp`**:

- `loadTexture()`: Full libpng decode using `setjmp`/`longjmp`; expands palette
  → RGB, grayscale → RGB, 16-bit → 8-bit, and adds alpha channel to produce
  always-RGBA output matching the game's texture expectations.
- `readAssetFile()`: Opens `asset_base_path_ + filename` (defaults to
  `./data/` relative to the binary).
- `getStoragePath()`: Returns `storage_path_ + filename` (defaults to
  `$HOME/.minecraft-pe/`).

---

## 6. Entry Point: main_linux.h

This header is `#include`d by `main.cpp` when `-DLINUX` is defined. `MAIN_CLASS`
is `NinecraftApp` (set in `main.cpp` before the include).

### Wayland vs. X11

SDL2 selects the backend at runtime:
- Default: X11 (if `DISPLAY` is set)
- Override: `SDL_VIDEODRIVER=wayland ./minecraft-pe`

No code changes are required to switch backends; SDL2 handles it transparently.

### GL context

```cpp
SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);
SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                    SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
```

The game uses the fixed-function OpenGL 1.x pipeline. A compatibility profile
at version 1.5 is requested. Mesa provides this via `MESA_GL_VERSION_OVERRIDE`
if needed.

### Key mapping — `linux_transform_key()`

The game's `Keyboard` class uses Windows Virtual Key codes internally:
- Letter keys: uppercase ASCII (A=65 … Z=90) — Win32 treats `VK_A` as `'A'`
- Digit keys: ASCII digit characters (0x30–0x39)
- Control: Backspace=8, Tab=9, Enter=13, Escape=27

SDL2 letter keycodes are **lowercase** ASCII (SDLK_a=97 … SDLK_z=122).
`linux_transform_key` converts them with `sym - 32`, producing the correct
uppercase value without any hardcoded table.

Special keys (F1-F12, arrows, Delete, Insert, …) are mapped explicitly to their
Win32 VK equivalents.

### `App::init` name-hiding fix

`NinecraftApp` declares a virtual `init()` with no arguments. This **hides** the
base class `App::init(AppContext&)` in derived-class lookup. The explicit
base-class call bypasses the ambiguity:

```cpp
app->App::init(context);  // not app->init(context)
```

---

## 7. World Name Input (SDL Text Input)

### Problem

Clicking "Create new world" opened a screen that immediately closed. The base
`AppPlatform::getUserInputStatus()` returned `0` (confirmed) on every tick.
`SelectWorldScreen::tick()` checked `if (status > -1)` which was always `true`,
resetting the input state before the user could type anything.

### Solution

`AppPlatform_linux` implements a non-blocking text input state machine backed by
SDL2's text-input subsystem:

```
createUserInput()
  └─ SDL_StartTextInput()
     text_input_status_ = -1  (waiting)

getUserInputStatus() → -1  (called every tick; no poll needed)

SDL_TEXTINPUT event → onTextInput()   → appends to text_input_buffer_
SDL_KEYDOWN Enter  → onTextConfirm()  → text_input_status_ = 1, SDL_StopTextInput()
SDL_KEYDOWN Escape → onTextCancel()   → text_input_status_ = 0
SDL_KEYDOWN Backspace → onTextBackspace() → removes last character
```

The event loop in `main_linux.h` routes SDL events to these handlers while
`isInTextInputMode()` is `true`, suppressing normal game key processing.

---

## 8. FPS Mouse Lock

### Design

Desktop FPS games lock the cursor to the window so mouse movement is used
entirely for camera look-around. SDL2's relative-mouse mode provides this:
`SDL_SetRelativeMouseMode(SDL_TRUE)` hides the cursor and delivers only delta
(`xrel`, `yrel`) motion, consuming unlimited mouse travel without hitting screen
edges.

The game's turn-input pipeline (`MouseTurnInput::MODE_DELTA`) calls
`Mouse::getDX()` / `Mouse::getDY()`. `Mouse::getDX()` returns the stored `_dx`
field when it is not `DELTA_NOTSET`. The six-argument `Mouse::feed(..., dx, dy)`
sets `_dx` and `_dy` directly. The event loop was already passing
`event.motion.xrel` / `event.motion.yrel` into that call, so the pipeline works
correctly in relative mode with no further changes inside the game.

### State machine

```
Initial state: mouse NOT grabbed (cursor visible, menus are clickable)

Left-click (while not grabbed):
  → SDL_SetRelativeMouseMode(SDL_TRUE)
  → g_linux_mouse_grabbed = true
  → The triggering click is not forwarded to the game to avoid accidental action.

Escape key (while grabbed):
  → SDL_SetRelativeMouseMode(SDL_FALSE)
  → g_linux_mouse_grabbed = false
  → Escape is still forwarded to the game → pause menu opens normally.
```

During the grab state, `SDL_MOUSEMOTION` reports the window centre as the
absolute cursor position (used only for UI hit-testing which cannot occur while
grabbed) and passes `xrel`/`yrel` as the turn deltas.

### Code location

`handheld/src/main_linux.h` — `linux_grab_mouse()` / `linux_release_mouse()`,
event handlers for `SDL_MOUSEMOTION`, `SDL_MOUSEBUTTONDOWN`, `SDL_KEYDOWN`.

---

## 9. AZERTY / ZQSD Layout

French AZERTY keyboards have Z and Q where QWERTY keyboards have W and A. The
game defaults to WASD movement. On Linux we override the defaults to ZQSD
immediately after constructing the `NinecraftApp` object, before `App::init()`.

```cpp
// handheld/src/main_linux.h — inside main(), after: MAIN_CLASS* app = new MAIN_CLASS();
app->options.keyUp.key   = Keyboard::KEY_Z;  // forward  (W→Z)
app->options.keyLeft.key = Keyboard::KEY_Q;  // strafe L (A→Q)
// keyDown (S=83) and keyRight (D=68) are unchanged; same key on both layouts.
```

`linux_transform_key()` already converts `SDLK_z` (122) to 90 (`'Z'`) and
`SDLK_q` (113) to 81 (`'Q'`) via the `sym - 32` letter mapping, so no
additional key-mapping changes are needed.

`Keyboard::KEY_Z` (90) and `Keyboard::KEY_Q` (81) are constants already
defined in `handheld/src/platform/input/Keyboard.h`.

---

## 10. Compiler Fixes

### RakNet narrowing conversions

GCC 15 in C++11 mode treats braced-init-list narrowing as a hard error.
RakNet uses patterns such as `char c = {0xFF}` (unsigned 255 → signed char is
narrowing). Fix: add `-fpermissive` to the RakNet-only compile options.

```cmake
target_compile_options(raknet PRIVATE -w -fpermissive)
```

### `NinecraftApp::init()` hiding `App::init(AppContext&)`

`NinecraftApp` declares:
```cpp
virtual void init();    // no-arg override
```

C++ name lookup finds this before the base `App::init(AppContext&)`, so a direct
`app->init(context)` call fails ("no matching function"). Fix: qualify with the
base class name.

```cpp
app->App::init(context);
```

---

## 11. File Inventory

### New files

| File | Description |
|------|-------------|
| `CMakeLists.txt` | Root CMake project |
| `CMakePresets.json` | `linux-release` / `linux-debug` presets |
| `cmake/CompilerFlags.cmake` | Compiler warning and optimisation flags |
| `cmake/FindDependencies.cmake` | Dependency discovery (epoxy, SDL2, OpenAL, PNG, ZLIB) |
| `handheld/project/linux/CMakeLists.txt` | `minecraft-pe` executable target |
| `handheld/src/AppPlatform_linux.h` | Linux platform interface |
| `handheld/src/AppPlatform_linux.cpp` | Linux platform implementation |
| `handheld/src/main_linux.h` | SDL2 entry point, event loop, key mapping |
| `scripts/setup_deps_linux.py` | Dependency installer (pacman / apt) |

### Modified files

| File | Change |
|------|--------|
| `handheld/src/client/renderer/gles.h` | `#include <epoxy/gl.h>` replaces GLEW + GL headers |
| `handheld/src/client/renderer/gl.h` | Same as above |
| `handheld/src/client/renderer/gl.cpp` | `glInit()` skips `glewInit()` on `#ifdef LINUX` |
| `handheld/src/main.cpp` | Added `#ifdef LINUX` / `#include "main_linux.h"` |
