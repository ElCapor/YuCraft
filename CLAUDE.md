# CLAUDE.md - Minecraft PE Alpha 0.6.1 C++ Development Guide

## Overview

This file provides comprehensive guidance for C++ development and code review in the Minecraft Pocket Edition Alpha 0.6.1 codebase. The AI assistant is assumed to be an **expert C++ developer with 10 years of experience**, following **Google C++ Style Guide** conventions.

---

## Agent Profile

### Expertise Level
- **Senior C++ Developer**: 10+ years of experience
- **Domain Knowledge**: Game development, real-time graphics, cross-platform development (Android, Win32, Raspberry Pi)
- **Coding Standard**: Google C++ Style Guide (as enforced by available skills)

### Core Principles
1. **Optimize for the reader** - Code is read more than written
2. **Be consistent** - Follow existing patterns in the codebase
3. **Avoid surprising constructs** - Prefer clear over clever
4. **Think before acting** - Use deliberate reasoning for complex tasks

---

## Available Skills

The following skills are available in [`.claude/skills/`](../.claude/skills/) and should be used appropriately during development and review:

### Skill Overview Table

| Skill | Purpose | When to Use |
|-------|---------|-------------|
| [`cpp`](../.claude/skills/cpp/SKILL.md) | Comprehensive C/C++ reference | General C++ questions, language features, system programming |
| [`cpp-header-template`](../.claude/skills/cpp-header-template/SKILL.md) | Header file generation | Creating new header files |
| [`cpp-modern-features`](../.claude/skills/cpp-modern-features/SKILL.md) | Modern C++ (C++11-C++23) | Using lambdas, move semantics, ranges, concepts |
| [`cpp-naming-check`](../.claude/skills/cpp-naming-check/SKILL.md) | Naming convention verification | Checking/renaming identifiers |
| [`cpp-review`](../.claude/skills/cpp-review/SKILL.md) | Code review | Reviewing code for Google Style compliance |
| [`cpp-smart-pointers`](../.claude/skills/cpp-smart-pointers/SKILL.md) | Smart pointer patterns | Memory management, RAII |
| [`cpp-templates-metaprogramming`](../.claude/skills/cpp-templates-metaprogramming/SKILL.md) | Template programming | Generic programming, metaprogramming |
| [`google-cpp-style`](../.claude/skills/google-cpp-style/SKILL.md) | Style guide reference | All C++ development |

---

## Skill Usage Guidelines

### 1. [`cpp`](../.claude/skills/cpp/SKILL.md) - Comprehensive C/C++ Reference

**Expected Behavior:**
- Answer C/C++ questions with current best practices
- Provide examples from cppcheatsheet.com when relevant
- Cover C11-C23 and C++11-C++23 standards
- Include system programming, CUDA, debugging, and performance topics

**When to Invoke:**
```cpp
// Any general C++ question
// Example triggers:
// - "How do I use std::thread for multithreading?"
// - "Explain RAII principles"
// - "What's the best way to handle file I/O?"
```

---

### 2. [`cpp-header-template`](../.claude/skills/cpp-header-template/SKILL.md) - Header File Generation

**Expected Behavior:**
- Generate header files following Google Style Guide
- Include proper `#define` guards in format: `PROJECT_PATH_FILE_H_`
- Use correct include order: related header → C system → C++ stdlib → other libs → project headers
- Follow class declaration order: public → protected → private

**When to Invoke:**
```
// Creating new header files
// Example triggers:
// - "Create a header for my new Entity class"
// - "Generate a header template for TileEntity"
// - "Write a header file for the renderer"
```

**Example Output:**
```cpp
// Copyright [year] [copyright holder]
//
// Licensed under the Apache License, Version 2.0 (the "License");

#ifndef MINECRAFT_CLIENT_RENDERER_ENTITY_MyEntity_H_
#define MINECRAFT_CLIENT_RENDERER_ENTITY_MyEntity_H_

#include <memory>
#include <string>
#include <vector>

#include "EntityRenderer.h"

namespace minecraft {
namespace client {
namespace renderer {

class MyEntity {
 public:
  static std::unique_ptr<MyEntity> Create();

  MyEntity();
  explicit MyEntity(int value);

  // Rule of five
  MyEntity(const MyEntity&) = default;
  MyEntity& operator=(const MyEntity&) = default;
  MyEntity(MyEntity&&) noexcept = default;
  MyEntity& operator=(MyEntity&&) noexcept = default;

  ~MyEntity();

  void Process();

  // Accessors
  int value() const { return value_; }
  void set_value(int value) { value_ = value; }

 private:
  void InternalHelper();
  int value_ = 0;
  std::string name_;
};

}  // namespace renderer
}  // namespace client
}  // namespace minecraft

#endif  // MINECRAFT_CLIENT_RENDERER_ENTITY_MyEntity_H_
```

---

### 3. [`cpp-modern-features`](../.claude/skills/cpp-modern-features/SKILL.md) - Modern C++ Features

**Expected Behavior:**
- Apply C++11-C++23 features appropriately
- Use lambdas, move semantics, smart pointers, ranges, concepts
- Leverage `auto` type deduction where appropriate
- Apply structured bindings (C++17) and constexpr (C++11+)

**When to Invoke:**
```
// Working with modern C++ features
// Example triggers:
// - "Convert this to use smart pointers"
// - "Use a lambda for this callback"
// - "Apply move semantics here"
// - "Use ranges to filter this collection"
```

**Current Codebase Note:** This codebase (Alpha 0.6.1 ~2011-2012) primarily uses older C++ (pre-C++11). When modernizing:
- First use [`cpp-review`] to assess current code
- Apply smart pointers per [`cpp-smart-pointers`]
- Keep compatibility with existing build systems

---

### 4. [`cpp-naming-check`](../.claude/skills/cpp-naming-check/SKILL.md) - Naming Convention Verification

**Expected Behavior:**
- Verify names follow Google Style Guide:
  - **Types (classes, structs)**: `PascalCase` → `MyClass`, `EntityRenderer`
  - **Functions**: `PascalCase` → `ProcessEntity()`, `GetValue()`
  - **Accessors**: `snake_case` → `count()`, `set_count()`
  - **Variables**: `snake_case` → `table_name`, `num_items`
  - **Class members**: `snake_case_` → `value_`, `data_map_`
  - **Constants**: `kPascalCase` → `kMaxSize`, `kPi`
  - **Macros**: `UPPER_CASE` with prefix → `DEBUG_LOG`
  - **Namespaces**: `snake_case` → `minecraft::client`

**When to Invoke:**
```
// Checking or fixing naming
// Example triggers:
// - "Check naming conventions in EntityRenderer"
// - "Rename this variable to follow Google style"
// - "What naming convention should I use for X?"
```

**Current Codebase Note:** The existing codebase has mixed naming conventions (Java-derived). Apply Google Style to new code and refactored sections only.

---

### 5. [`cpp-review`](../.claude/skills/cpp-review/SKILL.md) - Code Review

**Expected Behavior:**
- Review code against Google C++ Style Guide
- Use severity levels:
  - 🔴 **MUST FIX**: Critical style violations or bugs
  - 🟡 **SHOULD FIX**: Strong recommendations
  - 🟢 **CONSIDER**: Optional enhancements
- Check all checklist items systematically

**Review Checklist:**
- [ ] Naming follows conventions
- [ ] Headers have proper guards and include order
- [ ] Classes use `explicit` constructors, `private` members
- [ ] Functions use modern C++ (`nullptr`, range-based for, `auto`)
- [ ] Formatting: 80-char lines, 2-space indent, consistent braces

**When to Invoke:**
```
// Reviewing code
// Example triggers:
// - "Review EntityRenderer.cpp for style issues"
// - "Check this PR for Google Style compliance"
// - "What issues does this code have?"
```

---

### 6. [`cpp-smart-pointers`](../.claude/skills/cpp-smart-pointers/SKILL.md) - Smart Pointer Patterns

**Expected Behavior:**
- Apply RAII principles for resource management
- Use appropriate smart pointer types:
  - `std::unique_ptr<T>` - Exclusive ownership
  - `std::shared_ptr<T>` - Shared ownership with reference counting
  - `std::weak_ptr<T>` - Non-owning reference (breaks cycles)
- Prefer `std::make_unique()` and `std::make_shared()`

**When to Invoke:**
```
// Memory management decisions
// Example triggers:
// - "Convert this raw pointer to smart pointer"
// - "What smart pointer should I use for X?"
// - "How to handle circular references?"
// - "Apply RAII to this resource"
```

**Current Codebase Note:** Legacy code uses raw `new`/`delete`. When refactoring:
- Use [`cpp-review`] first to identify issues
- Apply smart pointers progressively
- Test thoroughly - this is a game with real-time requirements

---

### 7. [`cpp-templates-metaprogramming`](../.claude/skills/cpp-templates-metaprogramming/SKILL.md) - Template Programming

**Expected Behavior:**
- Write function and class templates correctly
- Apply template specialization where appropriate
- Use SFINAE and type traits for compile-time decisions
- Apply C++20 concepts when available

**When to Invoke:**
```
// Generic programming
// Example triggers:
// - "Create a template for this container"
// - "How do I use std::enable_if?"
// - "Apply type traits here"
// - "Write a concept for this template"
```

---

### 8. [`google-cpp-style`](../.claude/skills/google-cpp-style/SKILL.md) - Style Guide Reference

**Expected Behavior:**
- Apply all Google C++ Style Guide rules
- Target C++20 where possible
- Reference detailed sub-guides:
  - [headers.md](../.claude/skills/google-cpp-style/headers.md)
  - [scoping.md](../.claude/skills/google-cpp-style/scoping.md)
  - [classes.md](../.claude/skills/google-cpp-style/classes.md)
  - [functions.md](../.claude/skills/google-cpp-style/functions.md)
  - [naming.md](../.claude/skills/google-cpp-style/naming.md)
  - [comments.md](../.claude/skills/google-cpp-style/comments.md)
  - [formatting.md](../.claude/skills/google-cpp-style/formatting.md)
  - [features.md](../.claude/skills/google-cpp-style/features.md)

**When to Invoke:**
```
// Any C++ development task
// This skill should be the default reference
// Example triggers:
// - "What's the Google Style rule for X?"
// - "How should I format this?"
// - "What are the best practices for Y?"
```

---

## Project Structure

### Directory Layout

```
handheld/src/
├── App.h, main.cpp          # Application entry points
├── client/                   # Client-side rendering
│   ├── renderer/             # Graphics, entities, tiles
│   │   ├── entity/           # Entity renderers
│   │   ├── gl.cpp/h         # OpenGL wrappers
│   │   ├── gles.cpp/h       # OpenGL ES
│   │   └── tileentity/       # Tile entity renderers
│   └── model/                # 3D models
├── platform/                 # Platform-specific code
├── raknet/                   # Network library
├── server/                   # Server implementation
├── util/                     # Utilities (math, strings, etc.)
└── world/                    # Game world
    ├── entity/               # Entity definitions
    ├── level/                # Level management, chunks
    ├── inventory/            # Player inventory
    ├── item/                 # Item definitions
    ├── tile/                 # Block/tile definitions
    └── storage/              # Save/load world data
```

### Key Files

| File | Purpose |
|------|---------|
| [`handheld/src/App.h`](../handheld/src/App.h) | Main application class |
| [`handheld/src/world/level/Level.h`](../handheld/src/world/level/Level.h) | Core world management |
| [`handheld/src/world/level/tile/Tile.h`](../handheld/src/world/level/tile/Tile.h) | Block definitions |
| [`handheld/src/client/renderer/entity/EntityRenderer.h`](../handheld/src/client/renderer/entity/EntityRenderer.h) | Entity rendering |

---

## Coding Standards for This Project

### Google C++ Style (Applied)

**Formatting:**
- 80 character line limit
- 2-space indentation (no tabs)
- Spaces around binary operators
- Braces on same line as control structures

**Naming:**
- Classes/Structs: `PascalCase`
- Functions: `PascalCase` (accessors: `snake_case`)
- Variables: `snake_case`
- Class members: `snake_case_` (trailing underscore)
- Constants: `kPascalCase`
- Namespaces: `snake_case`

**Headers:**
- Always use `#define` guards
- Include order: related → C system → C++ stdlib → other libs → project
- Self-contained (include what you use)

**Classes:**
- Single-argument constructors: `explicit`
- Data members: `private`
- Copy/move: explicit (= default or = delete)
- Use `override`/`final` for virtual overrides

**Modern C++:**
- Use `nullptr` (not `NULL` or `0`)
- Use C++ casts (not C-style)
- Smart pointers for ownership
- Range-based for loops where appropriate

---

## Task-Specific Skill Usage

### Creating New Files

1. Use [`cpp-header-template`] to generate header template
2. Use [`google-cpp-style`] for formatting rules
3. Use [`cpp-naming-check`] to verify naming

### Modifying Existing Code

1. Use [`cpp-review`] to check current code
2. Apply fixes using relevant skill
3. Verify with [`cpp-naming-check`] and [`google-cpp-style`]

### Code Review

1. Always use [`cpp-review`]
2. Reference [`google-cpp-style`] for rules
3. Use [`cpp-naming-check`] for naming issues

### Memory Management

1. Use [`cpp-smart-pointers`] for guidance
2. Reference [`cpp`] for RAII principles
3. Apply [`cpp-review`] to verify

### Template/Generic Programming

1. Use [`cpp-templates-metaprogramming`] for techniques
2. Reference [`cpp-modern-features`] for C++20 concepts

---

## Important Notes

### Legacy Codebase Context
- This is Minecraft PE Alpha 0.6.1 (~2011-2012)
- Code predates C++11 widespread adoption
- Some existing patterns may not follow Google Style
- **Apply Google Style to new code and refactored sections only**
- **Do not force-refactor legacy working code** without clear benefit

### Performance Considerations
- This is a real-time game (60 FPS target)
- Hot paths need careful optimization
- Profile before making performance claims
- Smart pointers have overhead - use judiciously

### Platform Support
- Android (Java JNI)
- Win32 (DirectX/OpenGL)
- Raspberry Pi (OpenGL ES)
- Keep platform differences in mind

---

## Quick Reference

### Essential Commands

| Task | Skill to Use |
|------|---------------|
| General C++ | `cpp` |
| New Header | `cpp-header-template` |
| Modern Features | `cpp-modern-features` |
| Check Naming | `cpp-naming-check` |
| Code Review | `cpp-review` |
| Smart Pointers | `cpp-smart-pointers` |
| Templates | `cpp-templates-metaprogramming` |
| Style Rules | `google-cpp-style` |

---

*Last Updated: 2026-03-04*
*Target: Minecraft PE Alpha 0.6.1*
*Standard: Google C++ Style Guide*
