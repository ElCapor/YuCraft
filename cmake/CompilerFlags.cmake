# Compiler flags for Minecraft PE Alpha 0.6.1 Linux build

# ── Warning flags ──────────────────────────────────────────────────────────
set(MCPE_WARNING_FLAGS
  -Wall
  -Wextra
  -Wpedantic
  -Wno-unused-parameter      # Codebase has many unused params (legacy)
  -Wno-sign-compare          # Widespread signed/unsigned comparisons in game code
  -Wno-deprecated-declarations
  -Wno-missing-field-initializers
)

# ── Optimization flags ─────────────────────────────────────────────────────
set(MCPE_RELEASE_FLAGS
  -O2
  -DNDEBUG
)

set(MCPE_DEBUG_FLAGS
  -O0
  -g3
  -DDEBUG
  -fno-omit-frame-pointer
  -fsanitize=address,undefined  # Enable sanitizers in debug builds
)

# ── Language flags ─────────────────────────────────────────────────────────
set(MCPE_COMPILE_OPTIONS
  -fPIC
  -Wno-narrowing              # Game code has many implicit narrowing conversions
  -Wno-reorder                # Game code has constructor init list reordering
  -fvisibility=hidden
)

# ── Apply flags helper ─────────────────────────────────────────────────────
function(mcpe_set_compiler_flags target)
  target_compile_options(${target} PRIVATE
    ${MCPE_WARNING_FLAGS}
    ${MCPE_COMPILE_OPTIONS}
    $<$<CONFIG:Debug>:${MCPE_DEBUG_FLAGS}>
    $<$<CONFIG:Release>:${MCPE_RELEASE_FLAGS}>
    $<$<CONFIG:RelWithDebInfo>:-O2 -g -DNDEBUG>
  )

  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_link_options(${target} PRIVATE
      -fsanitize=address,undefined
    )
  endif()
endfunction()
