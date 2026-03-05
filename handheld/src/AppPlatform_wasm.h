#ifndef APPPLATFORM_WASM_H__
#define APPPLATFORM_WASM_H__

#include "AppPlatform.h"
#include "platform/log.h"

#include <emscripten.h>
#include <png.h>
#include <cmath>
#include <fstream>
#include <sstream>
#include <string>

// libpng read callback — same pattern as AppPlatform_linux.
static void png_read_callback_wasm(png_structp png_ptr,
                                   png_bytep data,
                                   png_size_t length) {
    static_cast<std::istream*>(png_get_io_ptr(png_ptr))
        ->read(reinterpret_cast<char*>(data),
               static_cast<std::streamsize>(length));
}

class AppPlatform_wasm : public AppPlatform {
 public:
    AppPlatform_wasm();

    // Set by main_wasm.h after SDL2 window creation.
    void SetScreenDims(int width, int height) {
        screen_width_  = width;
        screen_height_ = height;
    }

    // ── AppPlatform overrides ──────────────────────────────────────────────

    TextureData loadTexture(const std::string& filename,
                            bool texture_folder) override;

    BinaryBlob  readAssetFile(const std::string& filename) override;

    void saveScreenshot(const std::string& /*filename*/,
                        int /*gl_width*/, int /*gl_height*/) override {}

    void playSound(const std::string& /*name*/,
                   float /*volume*/, float /*pitch*/) override {}

    std::string getDateString(int s) override;

    int   checkLicense()                   override { return 0; }
    bool  hasBuyButtonWhenInvalidLicense() override { return false; }

    int   getScreenWidth()  override { return screen_width_;  }
    int   getScreenHeight() override { return screen_height_; }

    // ~90 DPI (typical monitor) / 25.4 mm per inch ≈ 3.5 px/mm.
    // Can be refined via the devicePixelRatio JS API if needed.
    float getPixelsPerMillimeter() override { return 3.5f; }

    // The browser canvas can receive touch events.
    bool supportsTouchscreen() override { return true; }

    // ── World-name dialog ─────────────────────────────────────────────────
    // Browser cannot display a synchronous modal prompt from C++.
    // We immediately confirm with the default name "World".
    // A JS interop implementation (via emscripten_run_script / EM_ASM) could
    // provide a real async dialog, but that requires ASYNCIFY and is out of
    // scope for the build system layer.
    void         createUserInput() override;
    int          getUserInputStatus() override;
    StringVector getUserInput() override;

 private:
    // Assets are embedded into Emscripten's virtual FS via --preload-file:
    //   --preload-file handheld/data@/data  (CMake)  →  /data/ at runtime
    std::string asset_base_path_{"/data/"};

    // Worlds and settings live in /minecraft-pe/ (Emscripten MemFS).
    // Mount IDBFS at this path for persistence across page reloads.
    std::string storage_path_{"/minecraft-pe/"};

    int screen_width_{854};
    int screen_height_{480};

    // getUserInputStatus() returns 1 (confirmed) immediately after
    // createUserInput() is called, with "World" as the world name.
    int  text_input_status_{-1};
    std::string text_input_buffer_{"World"};
};

#endif  // APPPLATFORM_WASM_H__
