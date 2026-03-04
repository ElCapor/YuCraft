#!/usr/bin/env python3
"""
setup_deps_linux.py — Install Minecraft PE build dependencies on Linux.

Supports: Arch Linux / Manjaro, Debian / Ubuntu / Mint, Fedora / RHEL 8+.

Usage:
    python3 scripts/setup_deps_linux.py [--dry-run]

Flags:
    --dry-run   Print commands that would be run without executing them.
"""

import os
import platform
import subprocess
import sys
from pathlib import Path

DRY_RUN = "--dry-run" in sys.argv

# ── Required packages per distro ───────────────────────────────────────────

PACKAGES = {
    "arch": [
        "cmake",
        "gcc",
        "make",
        "pkgconf",
        "sdl2",
        "glew",
        "openal",
        "libpng",
        "zlib",
        "mesa",            # OpenGL runtime
        "libx11",          # X11 backend for SDL2
        "wayland",         # Wayland backend for SDL2
        "wayland-protocols",
    ],
    "debian": [
        "cmake",
        "gcc",
        "g++",
        "make",
        "pkg-config",
        "libsdl2-dev",
        "libglew-dev",
        "libopenal-dev",
        "libpng-dev",
        "zlib1g-dev",
        "libgl-dev",       # OpenGL headers
        "libx11-dev",
        "libwayland-dev",
        "wayland-protocols",
    ],
    "fedora": [
        "cmake",
        "gcc-c++",
        "make",
        "pkg-config",
        "SDL2-devel",
        "glew-devel",
        "openal-soft-devel",
        "libpng-devel",
        "zlib-devel",
        "mesa-libGL-devel",
        "libX11-devel",
        "wayland-devel",
        "wayland-protocols-devel",
    ],
}

# ── Distro detection ───────────────────────────────────────────────────────

def detect_distro() -> str:
    """Return 'arch', 'debian', 'fedora', or raise RuntimeError."""
    if Path("/etc/arch-release").exists():
        return "arch"

    os_release = Path("/etc/os-release")
    if os_release.exists():
        info = {}
        for line in os_release.read_text().splitlines():
            if "=" in line:
                key, _, value = line.partition("=")
                info[key.strip()] = value.strip().strip('"')

        distro_id      = info.get("ID", "").lower()
        distro_id_like = info.get("ID_LIKE", "").lower()

        if "arch" in (distro_id, distro_id_like):
            return "arch"
        if any(d in (distro_id, distro_id_like)
               for d in ("debian", "ubuntu", "linuxmint", "pop")):
            return "debian"
        if any(d in (distro_id, distro_id_like)
               for d in ("fedora", "rhel", "centos", "rocky", "almalinux")):
            return "fedora"

    raise RuntimeError(
        "Could not detect Linux distribution.\n"
        "Supported: Arch/Manjaro, Debian/Ubuntu/Mint, Fedora/RHEL.\n"
        "Manually install: cmake gcc sdl2 glew openal libpng zlib mesa"
    )


# ── Package manager invocations ────────────────────────────────────────────

INSTALL_COMMANDS = {
    "arch":   ["sudo", "pacman", "-S", "--needed", "--noconfirm"],
    "debian": ["sudo", "apt-get", "install", "-y"],
    "fedora": ["sudo", "dnf",     "install", "-y"],
}

UPDATE_COMMANDS = {
    "arch":   ["sudo", "pacman", "-Sy"],
    "debian": ["sudo", "apt-get", "update"],
    "fedora": [],  # dnf handles this implicitly
}


def run(cmd: list[str]) -> None:
    print("  $", " ".join(cmd))
    if not DRY_RUN:
        result = subprocess.run(cmd)
        if result.returncode != 0:
            sys.exit(f"Command failed with exit code {result.returncode}")


def check_cmake_version() -> None:
    """Warn if the installed CMake is older than 3.20."""
    try:
        out = subprocess.check_output(["cmake", "--version"],
                                      stderr=subprocess.DEVNULL,
                                      text=True)
        version_line = out.splitlines()[0]  # "cmake version 3.x.y"
        parts = version_line.split()
        if len(parts) >= 3:
            major, minor, *_ = parts[2].split(".")
            if (int(major), int(minor)) < (3, 20):
                print(
                    f"\n[WARN] CMake {parts[2]} found, but >= 3.20 is required.\n"
                    "       Install a newer version from https://cmake.org/download/\n"
                    "       or use: pip install cmake"
                )
    except (FileNotFoundError, subprocess.CalledProcessError, ValueError):
        pass  # cmake not installed yet — installer will add it


def verify_sdl2_wayland() -> None:
    """Check that the installed SDL2 was built with Wayland support."""
    try:
        out = subprocess.check_output(
            ["sdl2-config", "--version"],
            stderr=subprocess.DEVNULL,
            text=True,
        ).strip()
        print(f"\n[OK] SDL2 {out} found.")
        print(
            "     To use Wayland backend, launch the game with:\n"
            "       SDL_VIDEODRIVER=wayland ./minecraft-pe"
        )
    except FileNotFoundError:
        pass


# ── Main ───────────────────────────────────────────────────────────────────

def main() -> None:
    if platform.system() != "Linux":
        sys.exit("This script only runs on Linux.")

    if DRY_RUN:
        print("[DRY RUN] No commands will be executed.\n")

    print("Detecting Linux distribution…")
    distro = detect_distro()
    print(f"  Detected: {distro}\n")

    print("Updating package database…")
    update_cmd = UPDATE_COMMANDS[distro]
    if update_cmd:
        run(update_cmd)

    packages = PACKAGES[distro]
    print(f"\nInstalling {len(packages)} packages:")
    for pkg in packages:
        print(f"  - {pkg}")
    print()

    install_cmd = INSTALL_COMMANDS[distro] + packages
    run(install_cmd)

    print("\nChecking CMake version…")
    check_cmake_version()

    if not DRY_RUN:
        verify_sdl2_wayland()

    print("\n✓ Dependencies installed successfully!")
    print(
        "\nNext steps:\n"
        "  1. cmake -B build -DCMAKE_BUILD_TYPE=Release\n"
        "  2. cmake --build build -j$(nproc)\n"
        "  3. cp -r handheld/data build/handheld/project/linux/data\n"
        "  4. cd build/handheld/project/linux && ./minecraft-pe\n"
        "\n"
        "To use the Wayland backend:\n"
        "  SDL_VIDEODRIVER=wayland ./minecraft-pe\n"
    )


if __name__ == "__main__":
    main()
