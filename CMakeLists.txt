cmake_minimum_required(VERSION 3.20)

project(MinecraftPEAlpha
    VERSION 0.6.1
    DESCRIPTION "Minecraft Pocket Edition Alpha 0.6.1 — Linux port"
    LANGUAGES CXX C
)

# ── Require C++17 as a minimum (codebase + POSIX helpers use C++03; C++17
#    allows structured bindings and if-init in new platform code) ──────────
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ── Default build type ─────────────────────────────────────────────────────
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING
        "Build type: Debug | Release | RelWithDebInfo" FORCE)
endif()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# ── Platform guard ─────────────────────────────────────────────────────────
if(NOT UNIX OR APPLE)
    message(FATAL_ERROR
        "This CMakeLists.txt is for Linux only.\n"
        "For Win32, open handheld/project/win32/MinecraftWin32.sln\n"
        "For Android, use the Android.mk / Gradle build in handheld/project/android/")
endif()

# ── Options ────────────────────────────────────────────────────────────────
option(MCPE_ENABLE_ASAN
    "Enable AddressSanitizer + UBSan (Debug builds only)"
    OFF)

option(MCPE_STRICT_WARNINGS
    "Treat all warnings as errors"
    OFF)

# ── cmake/ module path ─────────────────────────────────────────────────────
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# ── Linux client target ────────────────────────────────────────────────────
add_subdirectory(handheld/project/linux)

# ── Summary ────────────────────────────────────────────────────────────────
message(STATUS "")
message(STATUS "=== Minecraft PE Alpha 0.6.1 — Linux Build ===")
message(STATUS "  Build type  : ${CMAKE_BUILD_TYPE}")
message(STATUS "  Compiler    : ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "  Install dir : ${CMAKE_INSTALL_PREFIX}")
message(STATUS "")
message(STATUS "Quick start:")
message(STATUS "  cmake --build build -j\$(nproc)")
message(STATUS "  cd build/handheld/project/linux && ./minecraft-pe")
message(STATUS "")
message(STATUS "Wayland backend:")
message(STATUS "  SDL_VIDEODRIVER=wayland ./minecraft-pe")
message(STATUS "")
