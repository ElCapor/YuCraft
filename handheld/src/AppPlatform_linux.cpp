#include "AppPlatform_linux.h"
#include "world/level/storage/FolderMethods.h"

#include <SDL2/SDL.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <sstream>

// ── Constructor ────────────────────────────────────────────────────────────

AppPlatform_linux::AppPlatform_linux() {
    const char* home = std::getenv("HOME");
    if (home) {
        storage_path_ = std::string(home) + "/.minecraft-pe/";
    } else {
        storage_path_ = "./worlds/";
    }
}

// ── loadTexture ────────────────────────────────────────────────────────────

TextureData AppPlatform_linux::loadTexture(const std::string& filename_,
                                            bool texture_folder) {
    TextureData out;

    std::string filename = texture_folder
        ? asset_base_path_ + "images/" + filename_
        : filename_;

    std::ifstream source(filename.c_str(), std::ios::binary);
    if (!source) {
        LOGI("Couldn't find file: %s\n", filename.c_str());
        return out;
    }

    png_structp png_ptr = png_create_read_struct(
        PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) return out;

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        return out;
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        return out;
    }

    png_set_read_fn(png_ptr, static_cast<void*>(&source),
                    png_read_callback_linux);
    png_read_info(png_ptr, info_ptr);

    out.w = static_cast<int>(png_get_image_width(png_ptr,  info_ptr));
    out.h = static_cast<int>(png_get_image_height(png_ptr, info_ptr));

    // Expand to 8-bit RGBA regardless of source format.
    png_byte color_type = png_get_color_type(png_ptr, info_ptr);
    png_byte bit_depth  = png_get_bit_depth(png_ptr,  info_ptr);

    if (bit_depth == 16)
        png_set_strip_16(png_ptr);
    if (color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(png_ptr);
    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
        png_set_expand_gray_1_2_4_to_8(png_ptr);
    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(png_ptr);
    if (color_type == PNG_COLOR_TYPE_RGB   ||
        color_type == PNG_COLOR_TYPE_GRAY  ||
        color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_filler(png_ptr, 0xFF, PNG_FILLER_AFTER);
    if (color_type == PNG_COLOR_TYPE_GRAY ||
        color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb(png_ptr);

    png_read_update_info(png_ptr, info_ptr);

    out.memoryHandledExternally = false;
    out.data = new unsigned char[4 * out.w * out.h];

    png_bytep* row_ptrs = new png_bytep[out.h];
    const int row_stride = 4 * out.w;
    for (int i = 0; i < out.h; ++i) {
        row_ptrs[i] = reinterpret_cast<png_bytep>(&out.data[i * row_stride]);
    }

    png_read_image(png_ptr, row_ptrs);
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    delete[] row_ptrs;

    return out;
}

// ── User-input dialog ─────────────────────────────────────────────────────

void AppPlatform_linux::createUserInput() {
    text_input_active_ = true;
    text_input_status_ = -1;
    text_input_buffer_.clear();
    SDL_StartTextInput();
    LOGI("[Input] Type a world name and press Enter (Esc to cancel): ");
}

int AppPlatform_linux::getUserInputStatus() {
    return text_input_status_;
}

StringVector AppPlatform_linux::getUserInput() {
    StringVector sv;
    sv.push_back(text_input_buffer_);  // sv[0]=name; sv[1]=seed (omitted = random)
    return sv;
}

void AppPlatform_linux::onTextInput(const char* text) {
    if (!text_input_active_) return;
    text_input_buffer_ += text;
    LOGI("%s", text);  // echo typed character(s) to terminal
}

void AppPlatform_linux::onTextConfirm() {
    if (!text_input_active_) return;
    text_input_active_ = false;
    text_input_status_ = 1;
    SDL_StopTextInput();
    LOGI("\n[Input] World name: '%s'\n", text_input_buffer_.c_str());
}

void AppPlatform_linux::onTextCancel() {
    if (!text_input_active_) return;
    text_input_active_ = false;
    text_input_status_ = 0;
    SDL_StopTextInput();
    LOGI("\n[Input] Cancelled\n");
}

void AppPlatform_linux::onTextBackspace() {
    if (!text_input_active_ || text_input_buffer_.empty()) return;
    // Remove last UTF-8 codepoint (may be multi-byte)
    text_input_buffer_.pop_back();
    while (!text_input_buffer_.empty() &&
           (static_cast<unsigned char>(text_input_buffer_.back()) & 0xC0u) == 0x80u) {
        text_input_buffer_.pop_back();
    }
    LOGI("\r[Input] %s ", text_input_buffer_.c_str());
}

// ── readAssetFile ──────────────────────────────────────────────────────────

BinaryBlob AppPlatform_linux::readAssetFile(const std::string& filename) {
    std::string full_path = asset_base_path_ + filename;
    FILE* fp = std::fopen(full_path.c_str(), "rb");
    if (!fp) return BinaryBlob();

    long size = getRemainingFileSize(fp);
    if (size <= 0) {
        std::fclose(fp);
        return BinaryBlob();
    }

    BinaryBlob blob;
    blob.size = static_cast<int>(size);
    blob.data = new unsigned char[size];
    std::fread(blob.data, 1, static_cast<std::size_t>(size), fp);
    std::fclose(fp);
    return blob;
}

// ── saveScreenshot ─────────────────────────────────────────────────────────

void AppPlatform_linux::saveScreenshot(const std::string& /*filename*/,
                                        int /*gl_width*/,
                                        int /*gl_height*/) {
    // TODO: implement via glReadPixels + libpng
}

// ── getDateString ──────────────────────────────────────────────────────────

std::string AppPlatform_linux::getDateString(int s) {
    std::stringstream ss;
    ss << s << " s (UTC)";
    return ss.str();
}

// ── getPixelsPerMillimeter ─────────────────────────────────────────────────

float AppPlatform_linux::getPixelsPerMillimeter() {
    // Approximate for a 24-inch 1920×1080 display (~94 ppi → ~3.7 px/mm).
    // SDL_GetDisplayDPI() could provide a real value but requires knowing the
    // display index. Use a sensible fallback that matches common monitors.
    const float kDiagonalInches = 24.0f;
    const float kDiagonalPixels = std::sqrt(
        static_cast<float>(screen_width_  * screen_width_ +
                           screen_height_ * screen_height_));
    return kDiagonalPixels / (kDiagonalInches * 25.4f);
}
