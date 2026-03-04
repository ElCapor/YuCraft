#ifndef APPPLATFORM_LINUX_H__
#define APPPLATFORM_LINUX_H__

#include "AppPlatform.h"
#include "platform/log.h"
#include <png.h>
#include <cmath>
#include <fstream>
#include <sstream>
#include <string>

// libpng read callback (declared in the header so AppPlatform_linux.cpp
// does not need a separate translation-unit-local static for each include).
static void png_read_callback_linux(png_structp png_ptr,
                                    png_bytep data,
                                    png_size_t length) {
    static_cast<std::istream*>(png_get_io_ptr(png_ptr))
        ->read(reinterpret_cast<char*>(data),
               static_cast<std::streamsize>(length));
}

class AppPlatform_linux : public AppPlatform {
 public:
    AppPlatform_linux();

    // Called by main_linux.h once the SDL2 window is created.
    void SetScreenDims(int width, int height) {
        screen_width_  = width;
        screen_height_ = height;
    }

    // Path to the directory containing the "images/", "sound/", etc. folders.
    // Defaults to "./data/" (relative to the working directory).
    void setAssetBasePath(const std::string& path) { asset_base_path_ = path; }

    // Path where worlds and settings are stored (~/.minecraft-pe/ by default).
    void setStoragePath(const std::string& path) { storage_path_ = path; }

    // ── AppPlatform overrides ──────────────────────────────────────────────

    TextureData loadTexture(const std::string& filename,
                            bool texture_folder) override;

    BinaryBlob  readAssetFile(const std::string& filename) override;

    void saveScreenshot(const std::string& filename,
                        int gl_width, int gl_height) override;

    void playSound(const std::string& name,
                   float volume, float pitch) override {}

    std::string getDateString(int s) override;

    int   checkLicense() override               { return 0; }
    bool  hasBuyButtonWhenInvalidLicense()
              override                          { return false; }

    int   getScreenWidth()  override            { return screen_width_;  }
    int   getScreenHeight() override            { return screen_height_; }
    float getPixelsPerMillimeter() override;

    bool  supportsTouchscreen() override        { return false; }

 private:
    std::string asset_base_path_{"./data/"};
    std::string storage_path_;
    int screen_width_{1280};
    int screen_height_{720};
};

#endif  // APPPLATFORM_LINUX_H__
