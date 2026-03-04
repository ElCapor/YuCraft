# Dependency discovery for Minecraft PE Alpha 0.6.1 Linux build
#
# After inclusion, the following imported targets are available when found:
#   SDL2::SDL2          — window management, input, Wayland/X11 backend
#   OpenGL::GL          — desktop OpenGL
#   epoxy               — OpenGL extension loader (GLX + EGL / Wayland)
#   OpenAL::OpenAL      — audio
#   PNG::PNG            — texture loading
#   ZLIB::ZLIB          — NBT / region compression
#   Threads::Threads    — pthreads

cmake_minimum_required(VERSION 3.20)

# ── Threads ────────────────────────────────────────────────────────────────
find_package(Threads REQUIRED)

# ── OpenGL ─────────────────────────────────────────────────────────────────
find_package(OpenGL REQUIRED)
if(NOT OpenGL_FOUND)
  message(FATAL_ERROR
    "OpenGL not found.\n"
    "Install with: pacman -S mesa  OR  apt install libgl-dev")
endif()

# ── libepoxy ─────────────────────────────────────────────────────────────────────
# Epoxy transparently dispatches GL function pointers through GLX (X11) or
# EGL (Wayland) based on the active context — no glewInit() call needed.
find_package(PkgConfig REQUIRED)
pkg_check_modules(epoxy REQUIRED IMPORTED_TARGET epoxy)
if(NOT epoxy_FOUND)
  message(FATAL_ERROR
    "libepoxy not found.\n"
    "Install with: pacman -S libepoxy  OR  apt install libepoxy-dev")
endif()

# ── SDL2 ──────────────────────────────────────────────────────────────────
# Prefer the CMake config shipped with SDL2 >= 2.0.12; fall back to pkg-config.
find_package(SDL2 QUIET CONFIG)
if(NOT SDL2_FOUND)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2)
  # Provide a uniform imported target name
  if(NOT TARGET SDL2::SDL2)
    add_library(SDL2::SDL2 ALIAS PkgConfig::SDL2)
  endif()
endif()
if(NOT TARGET SDL2::SDL2 AND NOT TARGET SDL2::SDL2-static)
  message(FATAL_ERROR
    "SDL2 not found.\n"
    "Install with: pacman -S sdl2  OR  apt install libsdl2-dev")
endif()

# ── OpenAL ────────────────────────────────────────────────────────────────
find_package(OpenAL REQUIRED)
if(NOT OpenAL_FOUND AND NOT OPENAL_FOUND)
  message(FATAL_ERROR
    "OpenAL not found.\n"
    "Install with:\n"
    "  Arch/Manjaro : sudo pacman -S openal\n"
    "  Debian/Ubuntu: sudo apt install libopenal-dev\n"
    "  Fedora       : sudo dnf install openal-soft-devel")
endif()
# Normalise to an imported target when the legacy module is used
if(NOT TARGET OpenAL::OpenAL AND OPENAL_LIBRARY)
  add_library(OpenAL::OpenAL UNKNOWN IMPORTED)
  set_target_properties(OpenAL::OpenAL PROPERTIES
    IMPORTED_LOCATION "${OPENAL_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${OPENAL_INCLUDE_DIR}")
endif()

# ── libpng ────────────────────────────────────────────────────────────────
find_package(PNG REQUIRED)
if(NOT PNG_FOUND)
  message(FATAL_ERROR
    "libpng not found.\n"
    "Install with: pacman -S libpng  OR  apt install libpng-dev")
endif()

# ── zlib ──────────────────────────────────────────────────────────────────
find_package(ZLIB REQUIRED)
if(NOT ZLIB_FOUND)
  message(FATAL_ERROR
    "zlib not found.\n"
    "Install with: pacman -S zlib  OR  apt install zlib1g-dev")
endif()

# ── Summary ────────────────────────────────────────────────────────────────
message(STATUS "Dependencies found:")
message(STATUS "  OpenGL  : ${OPENGL_LIBRARIES}")
message(STATUS "  epoxy   : ${epoxy_LIBRARIES}")
message(STATUS "  SDL2    : found")
message(STATUS "  OpenAL  : ${OPENAL_LIBRARY}")
message(STATUS "  PNG     : ${PNG_LIBRARIES}")
message(STATUS "  ZLIB    : ${ZLIB_LIBRARIES}")
