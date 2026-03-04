# Minecraft PE Alpha 0.6.1 Modern C++ Rewrite Plan

## Executive Summary

This document outlines a comprehensive, agentic-AI-driven rewrite plan for transforming the Minecraft Pocket Edition Alpha 0.6.1 codebase from its legacy C++98/C++03 state into a modern, maintainable, cross-platform game engine using C++23. The rewrite prioritizes code quality, developer experience, and long-term extensibility while preserving the authentic gameplay mechanics of the original Alpha 0.6.1 version.

**Target Standard:** C++23 with modern C++ idioms
**Build System:** CMake 3.28+ with cross-platform support
**Target Platforms:** Linux (x64), Windows (x64)
**Style Guide:** Google C++ Style Guide (per CLAUDE.md)

---

## Table of Contents

1. [Architectural Overview](#1-architectural-overview)
2. [Module Design](#2-module-design)
3. [Code Transformation Rules](#3-code-transformation-rules)
4. [Build System Configuration](#4-build-system-configuration)
5. [Dependency Management](#5-dependency-management)
6. [Migration Order and Priorities](#6-migration-order-and-priorities)
7. [Coding Standards](#7-coding-standards)
8. [Testing Strategy](#8-testing-strategy)
9. [Tooling and Automation](#9-tooling-and-automation)
10. [Documentation Requirements](#10-documentation-requirements)

---

## 1. Architectural Overview

### 1.1 High-Level Architecture Diagram

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                              Application Layer                              │
│  ┌─────────────────┐  ┌─────────────────┐  ┌─────────────────────────────┐  │
│  │   Game Client   │  │  Game Server   │  │     Shared Library          │  │
│  │   (Renderer)    │  │  (Dedicated)   │  │  (Game Logic/Engine)        │  │
│  └────────┬────────┘  └────────┬────────┘  └──────────────┬─────────────┘  │
└───────────┼────────────────────┼─────────────────────────┼───────────────┘
            │                    │                         │
┌───────────┼────────────────────┼─────────────────────────┼───────────────┐
│           │              Platform Abstraction Layer    │                │
│  ┌────────┴────────┐  ┌────────┴────────┐  ┌───────────┴───────────┐    │
│  │   Win32/EGL    │  │   Linux/GLX    │  │   Platform Services   │    │
│  │   Input/Audio  │  │   Input/Audio  │  │   (File, Network,     │    │
│  │                │  │                │  │    Thread, Memory)    │    │
│  └────────────────┘  └────────────────┘  └───────────────────────┘    │
└────────────────────────────────────────────────────────────────────────┘
```

### 1.2 Core Engine Architecture

```
┌─────────────────────────────────────────────────────────────────────────┐
│                          Core Engine Modules                            │
│                                                                         │
│  ┌──────────────┐ ┌──────────────┐ ┌──────────────┐ ┌──────────────┐   │
│  │   Entity     │ │   World      │ │   Rendering  │ │   Physics    │   │
│  │   System     │ │   Generator  │ │   Engine     │ │   Engine     │   │
│  └──────────────┘ └──────────────┘ └──────────────┘ └──────────────┘   │
│                                                                         │
│  ┌──────────────┐ ┌──────────────┐ ┌──────────────┐ ┌──────────────┐   │
│  │  Inventory   │ │   Crafting   │ │     AI       │ │   Network    │   │
│  │   System    │ │    System    │ │   Behaviors  │ │   (RakNet)   │   │
│  └──────────────┘ └──────────────┘ └──────────────┘ └──────────────┘   │
│                                                                         │
│  ┌──────────────┐ ┌──────────────┐ ┌──────────────┐ ┌──────────────┐   │
│  │     Audio    │ │   Scripting  │ │    Asset     │ │    Save      │   │
│  │   Engine    │ │   (Optional) │ │   Manager    │ │   System     │   │
│  └──────────────┘ └──────────────┘ └──────────────┘ └──────────────┘   │
└─────────────────────────────────────────────────────────────────────────┘
```

### 1.3 Module Dependency Graph

```
                        ┌─────────────────┐
                        │   Entry Point   │
                        │   (main.cpp)    │
                        └────────┬────────┘
                                 │
                    ┌────────────┴────────────┐
                    │                         │
           ┌────────▼────────┐      ┌─────────▼────────┐
           │   Game Client   │      │   Game Server    │
           │   (Executable)  │      │   (Executable)   │
           └────────┬────────┘      └────────┬────────┘
                    │                        │
        ┌───────────┴───────────┐  ┌────────┴────────┐
        │                       │  │                │
 ┌──────▼──────┐      ┌────────▼──▼────────┐  ┌─────▼─────┐
 │   Renderer  │      │    Minecraft       │  │  RakNet   │
 │   Module    │      │    Core Engine    │  │  Module   │
 └──────┬──────┘      │    (Static Lib)  │  └─────┬─────┘
        │             └────────┬────────┘        │
        │                       │                  │
        │          ┌────────────┼────────────────┘
        │          │            │
        │   ┌──────▼──────┐ ┌───▼────┐
        │   │   Graphics   │ │ Input  │
        │   │   Services  │ │Service │
        │   └─────────────┘ └────────┘
        │
        │    ┌─────────────┐
        └───►│  Platform   │
             │  Abstraction│
             │   (Shared)  │
             └─────────────┘
```

---

## 2. Module Design

### 2.1 Core Engine Module (`minecraft-core`)

**Purpose:** Contains all game logic, data structures, and algorithms independent of platform or rendering.

**Location:** `src/core/`

```
minecraft-core/
├── include/minecraft/
│   ├── core/
│   │   ├── Common.hpp              # Common types, constants
│   │   ├── Version.hpp             # Version information
│   │   └── BuildConfig.hpp         # Build configuration
│   ├── entity/
│   │   ├── Entity.hpp              # Base entity class
│   │   ├── EntityManager.hpp       # Entity lifecycle management
│   │   ├── Player.hpp              # Player entity
│   │   ├── Mob.hpp                 # Base mob class
│   │   ├── PassiveMob.hpp          # Animals, villagers
│   │   ├── HostileMob.hpp         # Monsters, creepers
│   │   └── components/             # Entity component system
│   │       ├── HealthComponent.hpp
│   │       ├── PhysicsComponent.hpp
│   │       ├── InventoryComponent.hpp
│   │       └── AIComponent.hpp
│   ├── world/
│   │   ├── Level.hpp               # Main world container
│   │   ├── Chunk.hpp               # Chunk data structure
│   │   ├── Block.hpp               # Block definition
│   │   ├── BlockPosition.hpp       # 3D block coordinates
│   │   ├── ChunkColumn.hpp         # 16x16x128 column
│   │   ├── ChunkManager.hpp        # Chunk loading/unloading
│   │   ├── Generation/
│   │   │   ├── WorldGenerator.hpp  # Abstract generator
│   │   │   ├── FlatGenerator.hpp   # Flat world
│   │   │   ├── TerrainGenerator.hpp# Terrain generation
│   │   │   ├── Biome.hpp           # Biome system
│   │   │   └── Populator.hpp       # Feature placement
│   │   └── Storage/
│   │       ├── LevelStorage.hpp    # Save/load interface
│   │       ├── RegionFile.hpp      # Anvil region files
│   │       └── NBT.hpp             # Named Binary Tag
│   ├── physics/
│   │   ├── PhysicsWorld.hpp        # Physics simulation
│   │   ├── AABB.hpp               # Axis-aligned bounding box
│   │   ├── Collision.hpp          # Collision detection
│   │   └── RigidBody.hpp          # Physics object
│   ├── item/
│   │   ├── Item.hpp                # Base item class
│   │   ├── ItemStack.hpp          # Stack of items
│   │   ├── ItemRegistry.hpp       # Item registration
│   │   └── ItemTier.hpp           # Tool tier definitions
│   ├── inventory/
│   │   ├── Inventory.hpp          # Player inventory
│   │   ├── Container.hpp           # Generic container
│   │   ├── Slot.hpp               # Inventory slot
│   │   └── CraftingManager.hpp   # Recipe system
│   ├── crafting/
│   │   ├── Recipe.hpp             # Crafting recipe
│   │   ├── ShapedRecipe.hpp       # Shaped crafting
│   │   ├── ShapelessRecipe.hpp    # Shapeless crafting
│   │   ├── FurnaceRecipe.hpp      # Furnace recipes
│   │   └── RecipeRegistry.hpp     # Recipe database
│   ├── ai/
│   │   ├── AIComponent.hpp        # AI system component
│   │   ├── Goal.hpp               # AI goal interface
│   │   ├── GoalSelector.hpp       # Goal priority system
│   │   ├── behaviors/             # AI behavior modules
│   │   │   ├── WanderGoal.hpp
│   │   │   ├── AttackGoal.hpp
│   │   │   ├── FollowGoal.hpp
│   │   │   └── PanicGoal.hpp
│   │   └── paths/
│   │       ├── Pathfinding.hpp
│   │       └── Node.hpp
│   ├── util/
│   │   ├── Random.hpp             # Random number generation
│   │   ├── Math.hpp               # Math utilities
│   │   ├── String.hpp             # String utilities
│   │   ├── UUID.hpp               # Unique identifiers
│   │   └── Span.hpp               # Span utilities (C++20)
│   └── config/
│       ├── GameRules.hpp          # World game rules
│       └── Difficulty.hpp         # Difficulty settings
└── src/
    └── [Implementation files]
```

**Key Design Principles:**
- Use C++20/23 features: concepts, ranges, spans, constexpr algorithms
- RAII for all resources
- std::optional for nullable return types
- std::expected (C++23) for error handling
- Pimpl idiom where compilation time is critical

### 2.2 Rendering Module (`minecraft-renderer`)

**Purpose:** Cross-platform rendering abstraction with backend implementations.

**Location:** `src/renderer/`

```
minecraft-renderer/
├── include/minecraft/renderer/
│   ├── Renderer.hpp                # Main renderer interface
│   ├── Camera.hpp                  # View camera
│   ├── RenderContext.hpp          # Rendering context
│   ├── ShaderProgram.hpp           # Shader management
│   ├── TextureAtlas.hpp           # Texture management
│   ├── Mesh.hpp                   # Geometry mesh
│   ├── RenderBatch.hpp            # Batched rendering
│   ├── entities/
│   │   ├── EntityRenderer.hpp     # Entity rendering base
│   │   ├── HumanoidRenderer.hpp   # Player/mob rendering
│   │   └── ItemRenderer.hpp       # Item rendering
│   ├── tiles/
│   │   ├── TileRenderer.hpp       # Block rendering
│   │   └── BlockMesh.hpp          # Block geometry
│   └── ui/
│       ├── GuiRenderer.hpp        # GUI rendering
│       ├── FontRenderer.hpp       # Text rendering
│       └── HUDRenderer.hpp        # HUD elements
├── src/
│   ├── OpenGL/
│   │   ├── GLRenderer.hpp         # OpenGL backend
│   │   ├── GLShader.hpp           # OpenGL shaders
│   │   ├── GLTexture.hpp          # OpenGL textures
│   │   └── GLBuffer.hpp           # OpenGL buffers
│   └── Vulkan/
│       ├── VKRenderer.hpp         # Vulkan backend (future)
│       └── VKHelpers.hpp
└── shaders/                       # GLSL shader sources
    ├── terrain.vert
    ├── terrain.frag
    ├── entity.vert
    ├── entity.frag
    ├── gui.vert
    └── gui.frag
```

**Key Design Principles:**
- Abstract rendering API with concrete implementations
- Use modern pipeline: descriptor sets, push constants
- Modern OpenGL 4.6+/Vulkan for GPU efficiency
- Geometry batching for terrain rendering
- Frustum culling and level-of-detail (LOD)

### 2.3 Platform Abstraction Module (`minecraft-platform`)

**Purpose:** Platform-specific functionality abstracted for cross-platform compatibility.

**Location:** `src/platform/`

```
minecraft-platform/
├── include/minecraft/platform/
│   ├── Platform.hpp               # Main platform interface
│   ├── Window.hpp                 # Window management
│   ├── Input.hpp                 # Input handling
│   ├── Audio.hpp                 # Audio system
│   ├── FileSystem.hpp            # File operations
│   ├── Network.hpp               # Network sockets
│   ├── Thread.hpp                # Threading
│   ├── Time.hpp                  # Time functions
│   └── CrashHandler.hpp          # Crash reporting
├── src/
│   ├── common/                    # Platform-agnostic implementations
│   │   ├── CommonPlatform.cpp
│   │   └── DefaultAssets.cpp
│   ├── windows/
│   │   ├── WinWindow.hpp
│   │   ├── WinAudio.hpp
│   │   ├── WinFileSystem.hpp
│   │   └── WinNetwork.hpp
│   └── linux/
│       ├── LinuxWindow.hpp
│       ├── LinuxAudio.hpp
│       ├── LinuxFileSystem.hpp
│       └── LinuxNetwork.hpp
└── resources/
    └── [Platform-specific assets]
```

**Key Design Principles:**
- Factory pattern for platform object creation
- std::jthread for cancellable threading (C++20)
- Coroutines for async I/O
- Platform-agnostic error codes

### 2.4 Network Module (`minecraft-network`)

**Purpose:** Multiplayer networking based on modified RakNet.

**Location:** `src/network/`

```
minecraft-network/
├── include/minecraft/network/
│   ├── NetworkManager.hpp         # Main network controller
│   ├── RakNetBridge.hpp           # RakNet integration
│   ├── Packet.hpp                 # Network packet
│   ├── PacketHandler.hpp         # Packet processing
│   ├── Connection.hpp            # Player connection
│   ├── Server.hpp                # Server implementation
│   ├── Client.hpp                # Client implementation
│   └── protocols/
│       ├── LoginProtocol.hpp
│       ├── PlayProtocol.hpp
│       └── StatusProtocol.hpp
└── src/
    ├── RakNet/
    │   └── [Adapted RakNet source]
    └── [Network implementation]
```

**Key Design Principles:**
- Session-based connection management
- Packet serialization with version compatibility
- Client-side prediction and server reconciliation
- Keep original RakNet reliability for compatibility

### 2.5 Audio Module (`minecraft-audio`)

**Purpose:** Cross-platform audio playback.

**Location:** `src/audio/`

```
minecraft-audio/
├── include/minecraft/audio/
│   ├── AudioEngine.hpp            # Main audio controller
│   ├── SoundManager.hpp           # Sound loading/playback
│   ├── MusicManager.hpp          # Background music
│   ├── Sound.hpp                 # Sound instance
│   └── VoiceChat.hpp              # Voice communication (future)
└── src/
    ├── OpenAL/
    │   └── [OpenAL implementation]
    └── [Backend implementations]
```

---

## 3. Code Transformation Rules

### 3.1 Language Modernization Rules

| Legacy Pattern | Modern C++ Replacement | Example |
|--------------|----------------------|---------|
| `new`/`delete` | `std::unique_ptr`, `std::make_unique` | `new Widget()` → `std::make_unique<Widget>()` |
| `NULL` | `nullptr` | `NULL` → `nullptr` |
| C-style casts | `static_cast`, `reinterpret_cast` | `(int)x` → `static_cast<int>(x)` |
| `std::vector<T>::iterator` | `auto` | `std::vector<int>::iterator it` → `auto it` |
| Manual memory management | RAII, smart pointers | Raw pointer member → `std::unique_ptr<T>` |
| `enum` | `enum class` | `enum Color { RED }` → `enum class Color { Red }` |
| Variadic functions | Variadic templates | `printf()` → `std::format()` |
| Copy-on-write | Value semantics + move | Implicit sharing → Explicit move |
| `#define` constants | `constexpr`, `constinit` | `#define MAX 100` → `constexpr int kMax = 100;` |
| Manual reference counting | `std::shared_ptr` | Custom refcount → `std::shared_ptr<T>` |
| Public data members | Private + accessors | `struct Point { int x; };` → Class with getters |

### 3.2 Header Transformation Rules

**Before (Legacy):**
```cpp
// handheld/src/App.h
#ifndef APP_H__
#define APP_H__

#include <string>
#include "platform/log.h"

class App
{
public:
    App() : _finished(false), _inited(false) {}
    virtual ~App() {}

    void init(AppContext& c);
    bool isInited();
    virtual AppPlatform* platform();
    virtual void destroy();
    
private:
    bool _finished;
    bool _inited;
    AppContext _context;
};
#endif
```

**After (Modern C++23):**
```cpp
// include/minecraft/core/Application.hpp
#ifndef MINECRAFT_CORE_APPLICATION_HPP_
#define MINECRAFT_CORE_APPLICATION_HPP_

#include <memory>
#include <string_view>

#include "minecraft/core/PlatformFwd.hpp"
#include "minecraft/core/Version.hpp"

namespace minecraft {
namespace core {

/// Main application class for Minecraft PE.
/// Handles initialization, main loop, and shutdown.
class Application : public std::enable_shared_from_this<Application> {
 public:
  /// Constructs a new Application instance.
  Application();

  /// Destructor ensures proper cleanup.
  ~Application();

  // Disable copying - use shared_ptr
  Application(const Application&) = delete;
  Application& operator=(const Application&) = delete;

  // Allow moving
  Application(Application&&) noexcept = default;
  Application& operator=(Application&&) noexcept = default;

  /// Initializes the application with given context.
  /// @param context Platform-specific application context
  /// @returns true if initialization succeeded
  [[nodiscard]] bool Initialize(PlatformContext& context);

  /// Checks if application is fully initialized.
  /// @returns true if Initialize() succeeded
  [[nodiscard]] bool is_initialized() const noexcept { return is_initialized_; }

  /// Runs the main game loop.
  /// Blocks until application closes.
  void Run();

  /// Signals application to stop at next opportunity.
  void RequestShutdown() noexcept;

  /// Platform accessor.
  /// @returns Reference to platform services
  /// @pre is_initialized() == true
  [[nodiscard]] Platform& platform();

  /// Version information.
  [[nodiscard]] static constexpr Version version() noexcept { return Version::kCurrent; }

 private:
  void Shutdown();

  // State
  bool is_initialized_ = false;
  bool should_shutdown_ = false;

  // Owned resources
  std::unique_ptr<Platform> platform_;
  std::unique_ptr<class Game> game_;
};

}  // namespace core
}  // namespace minecraft

#endif  // MINECRAFT_CORE_APPLICATION_HPP_
```

### 3.3 Class Transformation Rules

**Rule 1: Explicit Constructors**
```cpp
// Single-argument constructors MUST be explicit
class Entity {
 public:
  explicit Entity(int id);  // Not implicit
  Entity(int x, int y);    // Multiple args - explicit optional but recommended
};
```

**Rule 2: Rule of Five/Zero**
```cpp
class ManagedResource {
 public:
  ManagedResource() = default;
  
  // Explicitly default or delete copy/move
  ManagedResource(const ManagedResource&) = default;
  ManagedResource& operator=(const ManagedResource&) = default;
  ManagedResource(ManagedResource&&) noexcept = default;
  ManagedResource& operator=(ManagedResource&&) noexcept = default;
  
  ~ManagedResource() = default;
  
 private:
  std::unique_ptr<int[]> data_;  // RAII - automatic cleanup
};
```

**Rule 3: Private Members with Accessors**
```cpp
class Level {
 public:
  // Public API
  [[nodiscard]] int get_seed() const noexcept { return seed_; }
  void set_seed(int seed) { seed_ = seed; }
  
 private:
  int seed_ = 0;
};
```

### 3.4 Naming Convention Transformations

| Legacy (Java-derived) | Google Style | Notes |
|----------------------|--------------|-------|
| `mValue` | `value_` | No Hungarian, use trailing underscore |
| `getValue()` | `value()` or `get_value()` | Accessors: snake_case preferred |
| `processData()` | `ProcessData()` | Functions: PascalCase |
| `MY_CONSTANT` | `kMyConstant` | Constants: kPascalCase |
| `className` | `class_name` | Variables: snake_case |
| `EnumValue` | `EnumValue` or `kEnumValue` | Enums: PascalCase or kPascalCase |
| `a_b` (struct) | `a_b` (struct) | Structs: no trailing underscore |
| `_private` | `private_` or `_member` | Leading underscore reserved |

### 3.5 Smart Pointer Usage Rules

```cpp
// Exclusive ownership - use unique_ptr
class Chunk {
  // ... members ...
 private:
  std::unique_ptr<TerrainData> terrain_data_;
};

// Shared ownership - use shared_ptr
class EntityRegistry {
 public:
  std::shared_ptr<Entity> CreateEntity() {
    return std::shared_ptr<Entity>(new Entity(), deleter_);
  }
  
 private:
  std::shared_ptr<EntityDeleter> deleter_;
};

// Weak references to break cycles
class Parent {
  std::weak_ptr<Child> child_;  // Doesn't own child
};
```

### 3.6 Modern C++23 Features Usage

**Concepts for Constraints:**
```cpp
#include <concepts>

template<typename T>
concept Numeric = std::integral<T> || std::floating_point<T>;

template<Numeric T>
T Add(T a, T b) { return a + b; }

// Better: use standard concepts
template<std::ranges::range Range>
auto Sum(Range&& range) {
  return std::accumulate(range.begin(), range.end(), std::ranges::range_value_t<Range>{});
}
```

**std::expected for Error Handling:**
```cpp
#include <expected>

[[nodiscard]] std::expected<Player*, Error> LoadPlayer(std::string_view name) {
  if (auto result = file_system_.Open(name); result.has_value()) {
    return ParsePlayer(*result);
  }
  return std::unexpected(Error::FileNotFound);
}

// Usage
if (auto player = LoadPlayer("player.dat")) {
  // Success
} else {
  // Handle error: player.error()
}
```

**std::span for Array Views:**
```cpp
void ProcessTiles(std::span<Tile> tiles) {
  for (auto& tile : tiles) {
    tile.Update();
  }
}

// Replaces: void ProcessTiles(Tile* tiles, size_t count)
```

**ranges:: for Algorithm Chaining:**
```cpp
#include <ranges>

auto visible_entities = entities_
  | std::views::filter([](const Entity& e) { return e.is_visible(); })
  | std::views::transform([](Entity& e) { return e.id(); })
  | std::views::take(100);
```

---

## 4. Build System Configuration

### 4.1 CMake Root Structure

```cmake
# CMakeLists.txt (root)
cmake_minimum_required(VERSION 3.28)
project(MinecraftPE VERSION 0.6.1 LANGUAGES CXX)

# C++23 standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for tooling
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Options
option(BUILD_SERVER "Build dedicated server" ON)
option(BUILD_CLIENT "Build game client" ON)
option(BUILD_TESTS "Build test suite" ON)
option(ENABLE_ASSERTS "Enable runtime assertions" ON)
option(ENABLE_DEBUG "Enable debug features" OFF)

# Dependencies
add_subdirectory(deps/glm)
add_subdirectory(deps/zlib)
# ... more dependencies

# Main targets
add_subdirectory(src/core)
add_subdirectory(src/platform)
add_subdirectory(src/renderer)
add_subdirectory(src/network)
add_subdirectory(src/audio)

if(BUILD_CLIENT)
  add_subdirectory(src/client)
endif()

if(BUILD_SERVER)
  add_subdirectory(src/server)
endif()

if(BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
```

### 4.2 Core Library CMake

```cmake
# src/core/CMakeLists.txt
add_library(minecraft-core STATIC)

target_sources(minecraft-core
  PRIVATE
    Entity/Entity.cpp
    Entity/Player.cpp
    World/Level.cpp
    World/Chunk.cpp
    World/Generation/TerrainGenerator.cpp
    # ... more sources
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include/minecraft/Common.hpp
)

target_include_directories(minecraft-core
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(minecraft-core
  PUBLIC
    zlib::zlib
)

# Compiler flags
target_compile_options(minecraft-core
  PRIVATE
    -Wall -Wextra -Wpedantic
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
)
```

### 4.3 Platform-Specific Configuration

```cmake
# src/platform/CMakeLists.txt
add_library(minecraft-platform STATIC)

# Detect platform
if(WIN32)
  set(PLATFORM_SOURCES
    src/windows/WinWindow.cpp
    src/windows/WinAudio.cpp
    src/windows/WinFileSystem.cpp
    src/windows/WinNetwork.cpp
  )
elseif(LINUX)
  set(PLATFORM_SOURCES
    src/linux/LinuxWindow.cpp
    src/linux/LinuxAudio.cpp
    src/linux/LinuxFileSystem.cpp
    src/linux/LinuxNetwork.cpp
  )
endif()

target_sources(minecraft-platform PRIVATE ${PLATFORM_SOURCES})

# Platform detection in code
target_compile_definitions(minecraft-platform
  PRIVATE
    $<$<BOOL:${WIN32}>:MINECRAFT_PLATFORM_WINDOWS=1>
    $<$<BOOL:${LINUX}>:MINECRAFT_PLATFORM_LINUX=1>
)
```

### 4.4 Client Executable CMake

```cmake
# src/client/CMakeLists.txt
add_executable(minecraft-client WIN32)

target_sources(minecraft-client
  PRIVATE
    main.cpp
    GameApp.cpp
    ClientMain.cpp
)

target_link_libraries(minecraft-client
  PRIVATE
    minecraft-core
    minecraft-renderer
    minecraft-platform
    minecraft-network
    minecraft-audio
)

# Windows entry point
if(WIN32)
  set_target_properties(minecraft-client PROPERTIES
    WIN32_EXECUTABLE ON
  )
endif()

# Install
install(TARGETS minecraft-client
  RUNTIME DESTINATION bin
)
```

### 4.5 Build Profiles

```cmake
# cmake/presets.cmake
{
  "configurePresets": [
    {
      "name": "release",
      "displayName": "Release",
      "binaryDir": "${sourceDir}/build/release",
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Release",
        "ENABLE_DEBUG": "OFF"
      }
    },
    {
      "name": "debug",
      "displayName": "Debug",
      "binaryDir": "${sourceDir}/build/debug",
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Debug",
        "ENABLE_DEBUG": "ON"
      }
    },
    {
      "name": "linux",
      "displayName": "Linux Release",
      "binaryDir": "${sourceDir}/build/linux",
      "toolchainFile": "${sourceDir}/cmake/linux.toolchain.cmake"
    }
  ]
}
```

---

## 5. Dependency Management

### 5.1 Internal Dependencies (Vendored)

| Library | Version | Purpose | Vendor Method |
|---------|---------|---------|---------------|
| zlib | 1.3 | Compression | git submodule |
| libpng | 1.6 | PNG loading | git submodule |

### 5.2 External Dependencies (System/Conan)

| Library | Version | Purpose | Manager |
|---------|---------|---------|---------|
| glm | 1.0+ | Math library | CMake FetchContent or conan |
| OpenAL | 1.0+ | Audio | System package |
| GLEW | 2.2+ | OpenGL extensions | System package |
| SDL2 | 2.28+ | Window/Input | System package or vendored |

### 5.3 CMake FetchContent Setup

```cmake
# cmake/FetchContent.cmake
include(FetchContent)

# GLM - Math library
FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG 1.0.1
)

# Google Test - Testing framework
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.14.0
)

#fmt - Formatting library (C++23 std::format fallback)
FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG 10.2.1
)

FetchContent_MakeAvailable(glm googletest fmt)
```

### 5.4 Dependency Graph

```
┌─────────────────────────────────────────────────────────────────┐
│                     Minecraft PE App                           │
├─────────────────────────────────────────────────────────────────┤
│  minecraft-client  │  minecraft-server  │  minecraft-tests    │
├────────────────────┼────────────────────┼─────────────────────┤
│  minecraft-renderer│  minecraft-core    │  google-test        │
│  minecraft-audio   │  minecraft-network │  minecraft-core     │
│  minecraft-platform│  minecraft-platform│  minecraft-platform│
├────────────────────┼────────────────────┼─────────────────────┤
│  minecraft-core    │  zlib              │  zlib               │
│  minecraft-network │  glm               │  glm                │
│  minecraft-platform│                    │  fmt                │
├────────────────────┼────────────────────┼─────────────────────┤
│  OpenAL, GLEW, SDL2 (platform-specific)                        │
└─────────────────────────────────────────────────────────────────┘
```

---

## 6. Migration Order and Priorities

### 6.1 Migration Phases

```
Phase 1: Foundation (Week 1-3)
├── 1.1 Set up CMake build system
├── 1.2 Create directory structure
├── 1.3 Implement Platform Abstraction Layer
│   ├── Window management
│   ├── Input system
│   └── File system
├── 1.4 Port basic types and utilities
│   ├── Math utilities (Vector3, Quaternion, Matrix)
│   ├── String utilities
│   └── Random number generation
└── 1.5 Verify empty shell compiles on both platforms

Phase 2: Core Engine (Week 4-8)
├── 2.1 Entity System
│   ├── Entity base class
│   ├── EntityManager
│   └── Player class
├── 2.2 World System
│   ├── Level class
│   ├── Chunk data structure
│   ├── Block definitions
│   └── ChunkManager
├── 2.3 World Generation
│   ├── TerrainGenerator
│   ├── Biome system
│   └── Populators
├── 2.4 Physics
│   ├── AABB collision
│   ├── Basic physics
│   └── Movement
└── 2.5 Save/Load System
    ├── NBT parsing
    ├── Region files
    └── Level storage

Phase 3: Rendering (Week 9-12)
├── 3.1 Rendering Framework
│   ├── Renderer abstraction
│   ├── Shader system
│   └── Texture system
├── 3.2 Terrain Rendering
│   ├── Chunk mesh generation
│   ├── Block rendering
│   └── Dynamic updates
├── 3.3 Entity Rendering
│   ├── Model loading
│   ├── Entity renderers
│   └── Animation
├── 3.4 GUI Rendering
│   ├── HUD
│   ├── Inventory UI
│   └── Menus
└── 3.5 Lighting and Effects
    ├── Ambient occlusion
    └── Fog

Phase 4: Gameplay Systems (Week 13-18)
├── 4.1 Inventory System
│   ├── Container classes
│   ├── Item definitions
│   └── Slot management
├── 4.2 Crafting System
│   ├── Recipe registry
│   ├── Crafting grid
│   └── Result calculation
├── 4.3 AI System
│   ├── AIComponent
│   ├── GoalSelector
│   └── Behavior goals
├── 4.4 Mob Behaviors
│   ├── Passive mobs
│   ├── Hostile mobs
│   └── Pathfinding
└── 4.5 Game Rules
    ├── Difficulty
    └── World settings

Phase 5: Networking (Week 19-22)
├── 5.1 Network Framework
│   ├── Packet system
│   ├── Connection management
│   └── Protocol handlers
├── 5.2 Server Integration
│   ├── Player connections
│   ├── World sync
│   └── Entity sync
└── 5.3 Client Integration
    ├── Server connection
    └── Client-side prediction

Phase 6: Audio (Week 23-24)
├── 6.1 Audio Engine
│   ├── Sound playback
│   ├── Music management
│   └── Spatial audio
└── 6.2 Sound Events
    ├── Block sounds
    ├── Mob sounds
    └── UI sounds

Phase 7: Polish & Integration (Week 25-28)
├── 7.1 Performance Optimization
│   ├── Profiling
│   └── Hot path optimization
├── 7.2 Testing
│   ├── Unit tests
│   ├── Integration tests
│   └── Gameplay tests
├── 7.3 Platform-Specific Polish
│   ├── Platform bugs
│   └── Performance tuning
└── 7.4 Documentation
    ├── API documentation
    └── Developer guides
```

### 6.2 Priority Matrix

| Component | Complexity | Dependencies | Priority |
|-----------|------------|--------------|----------|
| Platform Abstraction | High | None | P0 (Critical) |
| Core Types/Math | Medium | None | P0 (Critical) |
| Build System | High | None | P0 (Critical) |
| Entity System | High | Core Types | P1 (High) |
| World/Chunk | High | Entity, Core Types | P1 (High) |
| Physics | Medium | World | P1 (High) |
| Rendering | Very High | Platform | P1 (High) |
| Save/Load | Medium | World | P2 (Medium) |
| Inventory | Medium | Entity | P2 (Medium) |
| Crafting | Medium | Inventory | P2 (Medium) |
| AI | High | Entity, Physics | P2 (Medium) |
| Networking | Very High | Entity, World | P2 (Medium) |
| Audio | Medium | Platform | P3 (Low) |

---

## 7. Coding Standards

### 7.1 General Guidelines (per CLAUDE.md)

**Standard:** Google C++ Style Guide with C++23 extensions

**Key Rules:**
- 80 character line limit
- 2-space indentation (no tabs)
- Braces on same line as control structures
- Spaces around binary operators

### 7.2 File Organization

```cpp
// include/minecraft/module/ClassName.hpp

// 1. License header (for new files)
// Copyright 2024 Mojang AB
// Licensed under Apache 2.0

// 2. Include guard
#ifndef MINECRAFT_MODULE_CLASS_NAME_HPP_
#define MINECRAFT_MODULE_CLASS_NAME_HPP_

// 3. Module includes (alphabetical)
#include <memory>
#include <string>

// 4. Forward declarations (if needed)
class OtherClass;

// 5. Project includes
#include "minecraft/other/Header.hpp"

// 6. Namespace open
namespace minecraft {
namespace module {

// 7. Declarations

}  // namespace module
}  // namespace minecraft

#endif  // MINECRAFT_MODULE_CLASS_NAME_HPP_
```

### 7.3 Implementation File Organization

```cpp
// src/module/ClassName.cpp

// 1. Module header first (blank line after)
#include "minecraft/module/ClassName.hpp"

// 2. Other project headers
#include "minecraft/util/Logging.hpp"

// 3. System headers (if needed)
#include <cmath>

// 4. Namespace
namespace minecraft {
namespace module {

// 5. Implementation

}  // namespace module
}  // namespace minecraft
```

### 7.4 Comment Standards

```cpp
/// @brief Brief description of the function.
/// @details More detailed description if needed.
///          Can span multiple lines.
/// @param param1 Description of first parameter.
/// @param param2 Description of second parameter.
/// @returns Description of return value.
/// @throws std::invalid_argument if param1 is invalid.
/// @pre is_initialized() == true
/// @post Object is in a valid state.
[[nodiscard]] ReturnType FunctionName(Type1 param1, Type2 param2);

// Inline comment for implementation details
int calculate_something() {
  // Step 1: Initialize
  auto value = Initialize();
  
  // Step 2: Process (explain WHY, not WHAT)
  // We use squared distance to avoid expensive square root
  const float squared_distance = dx * dx + dy * dy;
  
  return value;
}
```

### 7.5 Modern C++ Patterns

```cpp
// Factory method pattern
class Entity : public std::enable_shared_from_this<Entity> {
 public:
  template<typename... Args>
  static std::unique_ptr<Entity> Create(Args&&... args) {
    return std::make_unique<Entity>(std::forward<Args>(args)...);
  }
  
  // For shared ownership
  static std::shared_ptr<Entity> CreateShared() {
    return std::shared_ptr<Entity>(new Entity());
  }
 private:
  Entity() = default;
};

// Builder pattern
class LevelBuilder {
 public:
  LevelBuilder& SetSeed(int seed) { seed_ = seed; return *this; }
  LevelBuilder& SetDifficulty(Difficulty diff) { difficulty_ = diff; return *this; }
  LevelBuilder& SetGenerator(GeneratorType type) { generator_ = type; return *this; }
  
  std::unique_ptr<Level> Build() {
    return std::make_unique<Level>(seed_, difficulty_, generator_);
  }
 private:
  int seed_ = 0;
  Difficulty difficulty_ = Difficulty::kNormal;
  GeneratorType generator_ = GeneratorType::kInfinite;
};

// Dependency injection
class Game {
 public:
  explicit Game(
    std::unique_ptr<World> world,
    std::unique_ptr<Renderer> renderer,
    std::unique_ptr<AudioEngine> audio
  ) : world_(std::move(world)),
      renderer_(std::move(renderer)),
      audio_(std::move(audio)) {}
 private:
  std::unique_ptr<World> world_;
  std::unique_ptr<Renderer> renderer_;
  std::unique_ptr<AudioEngine> audio_;
};
```

---

## 8. Testing Strategy

### 8.1 Test Pyramid

```
        ┌─────────────┐
        │     E2E    │  <-- Few, critical paths
        │   Tests    │
       ┌──────────────┐
       │ Integration  │  <-- Module interactions
       │    Tests     │
      ┌───────────────┐
      │   Unit Tests  │  <-- Many, fast, isolated
      └───────────────┘
```

### 8.2 Test Framework

**Framework:** Google Test (gtest)

**Additional:**
- Google Benchmark for performance testing
- Fuzz testing with libFuzzer

### 8.3 Test Structure

```
tests/
├── core/
│   ├── Entity_test.cpp
│   ├── Chunk_test.cpp
│   ├── Math_test.cpp
│   └── Level_test.cpp
├── world/
│   ├── Generation_test.cpp
│   ├── Biome_test.cpp
│   └── Storage_test.cpp
├── physics/
│   ├── Collision_test.cpp
│   └── AABB_test.cpp
├── inventory/
│   ├── Container_test.cpp
│   └── Crafting_test.cpp
├── rendering/
│   └── Renderer_test.cpp
└── integration/
    ├── GameLoop_test.cpp
    └── SaveLoad_test.cpp
```

### 8.4 Test Example

```cpp
// tests/core/Chunk_test.cpp
#include <gtest/gtest.h>

#include "minecraft/world/Chunk.hpp"

namespace minecraft {
namespace test {

using namespace ::testing;

class ChunkTest : public Test {
 protected:
  void SetUp() override {
    chunk_ = std::make_unique<Chunk>(ChunkPos{0, 0});
  }

  std::unique_ptr<Chunk> chunk_;
};

TEST_F(ChunkTest, DefaultConstruction) {
  EXPECT_EQ(chunk_->get_position().x, 0);
  EXPECT_EQ(chunk_->get_position().z, 0);
  EXPECT_FALSE(chunk_->is_loaded());
}

TEST_F(ChunkTest, SetBlock) {
  auto block = Block::Create<Block::Stone>();
  chunk_->SetBlock({0, 64, 0}, block);
  
  EXPECT_EQ(chunk_->GetBlock({0, 64, 0})->GetType(), BlockType::kStone);
}

TEST_F(ChunkTest, BlockOutOfBounds) {
  auto block = Block::Create<Block::Stone>();
  
  // Out of bounds should be ignored
  chunk_->SetBlock({0, 200, 0}, block);
  EXPECT_EQ(chunk_->GetBlock({0, 200, 0})->GetType(), BlockType::kAir);
}

}  // namespace test
}  // namespace minecraft
```

### 8.5 CI/CD Testing Pipeline

```yaml
# .github/workflows/test.yml
name: Tests

on: [push, pull_request]

jobs:
  test:
    runs-on: ${{ matrix.os }}
    strategy:
      matrix:
        os: [ubuntu-latest, windows-latest]
        compiler: [gcc, clang, msvc]
    
    steps:
      - uses: actions/checkout@v4
        with:
          submodules: recursive
      
      - name: Configure
        run: cmake -B build -DCMAKE_BUILD_TYPE=Release
      
      - name: Build
        run: cmake --build build -j$(nproc)
      
      - name: Run Unit Tests
        run: ctest --output-on-failure -R unit
      
      - name: Run Integration Tests
        run: ctest --output-on-failure -R integration

  benchmark:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      
      - name: Configure
        run: cmake -B build -DCMAKE_BUILD_TYPE=Release -DENABLE_BENCHMARKS=ON
      
      - name: Build
        run: cmake --build build -j$(nproc)
      
      - name: Run Benchmarks
        run: ./build/tests/benchmark --benchmark_format=json > benchmark.json
      
      - name: Upload Results
        uses: actions/upload-artifact@v4
        with:
          name: benchmark-results
          path: benchmark.json
```

---

## 9. Tooling and Automation

### 9.1 Required Tools

| Tool | Purpose | Configuration |
|------|---------|---------------|
| CMake 3.28+ | Build system | presets.json |
| C++23 Compiler | GCC 13+, Clang 17+, MSVC 2022+ | C++23 mode |
| clang-format | Code formatting | .clang-format |
| clang-tidy | Static analysis | .clang-tidy |
| cmake-format | CMake formatting | cmake-format.json |
| GDB/LLDB | Debugging | .gdbinit |

### 9.2 clang-format Configuration

```yaml
# .clang-format
BasedOnStyle: Google
Standard: cpp11
ColumnLimit: 80
IndentWidth: 2
TabWidth: 2
UseTab: Never
SpacesBeforeParens: ControlStatements
AllowShortFunctionsOnASingleLine: Inline
AllowShortIfStatementsOnASingleLine: Never
```

### 9.3 clang-tidy Configuration

```yaml
# .clang-tidy
Checks: >
  -*,
  bugprone-*,
  clang-analyzer-*,
  cppcoreguidelines-*,
  google-*,
  misc-*,
  modernize-*,
  performance-*,
  portability-*,
  -modernize-use-trailing-return-type,
  -cppcoreguidelines-avoid-magic-numbers
WarningsAsErrors: ''
HeaderFilterRegex: 'minecraft/.*'
```

### 9.4 Pre-commit Hook

```yaml
# .pre-commit-config.yaml
repos:
  - repo: https://github.com/pre-commit/pre-commit-hooks
    rev: v4.5.0
    hooks:
      - id: trailing-whitespace
      - id: end-of-file-fixer
      - id: check-yaml
      - id: check-added-large-files

  - repo: https://github.com/psf/black
    rev: 24.1.0
    hooks:
      - id: black
        language_version: python3.11

  - repo: https://github.com/pre-commit/mirrors-clang-format
    rev: v17.0.6
    hooks:
      - id: clang-format
        types: [c++]
```

### 9.5 AI Agent Guidelines

For autonomous AI agents performing the rewrite:

```markdown
# AI Agent Rewrite Guidelines

## Pre-Rewrite Checklist
- [ ] Analyze existing legacy code structure
- [ ] Identify all source files to transform
- [ ] Map dependencies between modules
- [ ] Create transformation rule mapping
- [ ] Set up build environment

## Per-File Transformation Rules
1. Create new file in appropriate module location
2. Apply naming convention transformations
3. Replace legacy patterns with modern C++23
4. Add comprehensive documentation comments
5. Ensure header self-containment
6. Run clang-format after transformation
7. Verify with clang-tidy

## Per-Module Validation
- [ ] All unit tests pass
- [ ] Module compiles on both platforms
- [ ] No clang-tidy warnings
- [ ] Code coverage maintained or improved

## Integration Testing
- [ ] Client builds successfully
- [ ] Server builds successfully
- [ ] Game launches without crash
- [ ] Basic gameplay functional
```

---

## 10. Documentation Requirements

### 10.1 Required Documentation

| Document | Location | Description |
|----------|----------|-------------|
| README.md | Root | Build instructions, quick start |
| ARCHITECTURE.md | docs/ | System architecture overview |
| MODULE_GUIDE.md | docs/ | Module-by-module documentation |
| CODING_STANDARDS.md | docs/ | Project-specific coding rules |
| BUILD_GUIDE.md | docs/ | Detailed build instructions |
| API_REFERENCE.md | docs/api/ | Auto-generated API docs |

### 10.2 API Documentation (Doxygen)

```cpp
/// @file
/// @brief Chunk data structure for world storage.
///
/// The Chunk class represents a 16x16x128 column of blocks.
/// Chunks are the fundamental unit of world storage and rendering.
///
/// @note Chunk coordinates are in block space, not chunk space.
///       Use ChunkPosition for chunk-relative coordinates.
///
/// @see Level
/// @see ChunkManager
class Chunk {
 public:
  /// Creates an empty chunk at the given position.
  ///
  /// @param position The chunk grid position (x, z)
  /// @param world The world this chunk belongs to
  Chunk(ChunkPosition position, World& world);

  /// Gets the block at the specified coordinates.
  ///
  /// @param pos The block position within this chunk
  /// @return Pointer to the block, or nullptr if out of bounds
  /// @note Thread-safe for concurrent reads
  [[nodiscard]] const Block* GetBlock(BlockPosition pos) const;

  /// Sets a block at the specified coordinates.
  ///
  /// @param pos The block position within this chunk
  /// @param block The block to set
  /// @return true if the block was successfully set
  /// @pre pos.y must be in range [0, 128)
  /// @post Chunk mesh is marked dirty if block changed
  bool SetBlock(BlockPosition pos, const Block& block);

  /// @name Chunk State
  ///@{
  
  /// Whether this chunk has been fully generated.
  [[nodiscard]] bool is_generated() const noexcept { return is_generated_; }
  
  /// Whether this chunk has been populated with decorations.
  [[nodiscard]] bool is_populated() const noexcept { return is_populated_; }
  
  /// Whether this chunk is currently loaded in memory.
  [[nodiscard]] bool is_loaded() const noexcept { return is_loaded_; }
  
  ///@}
  
 private:
  Block blocks_[kChunkWidth][kChunkHeight][kChunkDepth];
  ChunkPosition position_;
  World& world_;
  bool is_generated_ = false;
  bool is_populated_ = false;
  bool is_loaded_ = false;
};
```

---

## Appendix A: File Naming Convention

| Type | Pattern | Example |
|------|---------|---------|
| Header | `PascalCase.hpp` | `EntityManager.hpp` |
| Implementation | `PascalCase.cpp` | `EntityManager.cpp` |
| Test | `PascalCase_test.cpp` | `EntityManager_test.cpp` |
| Internal Header | `detail/Implementation.hpp` | `detail/ChunkPimpl.hpp` |

---

## Appendix B: Namespace Hierarchy

```
minecraft
├── core           # Core engine (entities, world, physics)
│   ├── entity
│   ├── world
│   │   ├── generation
│   │   └── storage
│   ├── physics
│   ├── item
│   ├── inventory
│   ├── crafting
│   ├── ai
│   │   ├── behaviors
│   │   └── paths
│   └── util
├── renderer       # Rendering subsystem
│   ├── opengl
│   ├── entities
│   ├── tiles
│   └── ui
├── platform       # Platform abstraction
│   ├── windows
│   └── linux
├── network        # Networking
│   └── protocols
├── audio          # Audio system
└── test           # Test utilities
```

---

## Appendix C: Migration Checklist

### Pre-Migration
- [ ] Set up CMake build system with all targets
- [ ] Create directory structure
- [ ] Verify empty shell builds on Windows and Linux

### During Migration
- [ ] Transform each module independently
- [ ] Maintain test coverage during transformation
- [ ] Verify cross-platform compilation after each module

### Post-Migration
- [ ] All tests passing
- [ ] No compiler warnings (warnings as errors)
- [ ] clang-tidy clean
- [ ] Documentation complete
- [ ] Performance regression testing

---

## Appendix D: Success Metrics

| Metric | Target | Measurement |
|--------|--------|-------------|
| Build Time | < 5 min | Full rebuild time |
| Code Coverage | > 80% | gcov/llvm-cov |
| Static Analysis | 0 warnings | clang-tidy |
| Platform Support | Win32, Linux | CI verification |
| Binary Size | < 50MB | Stripped release |
| Frame Rate | 60 FPS | Performance profiling |

---

*Document Version: 1.0*
*Last Updated: 2026-03-04*
*Target: Minecraft PE Alpha 0.6.1 Modern C++ Rewrite*
*Standard: Google C++ Style Guide, C++23*
