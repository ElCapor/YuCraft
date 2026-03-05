#include "AppPlatform_wasm.h"
#include "world/level/storage/FolderMethods.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <sstream>

// ── Constructor ────────────────────────────────────────────────────────────

AppPlatform_wasm::AppPlatform_wasm() {
    // Emscripten MemFS: no $HOME env var.  Use fixed paths in the virtual FS.
    // Mount IDBFS at /minecraft-pe/ in main_wasm.h for persistence if desired.
    storage_path_ = "/minecraft-pe/";
}

// ── loadTexture ────────────────────────────────────────────────────────────
// Identical logic to AppPlatform_linux::loadTexture; libpng is provided by
// Emscripten as a built-in port (-s USE_LIBPNG=1).

TextureData AppPlatform_wasm::loadTexture(const std::string& filename_,
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
                    png_read_callback_wasm);
    png_read_info(png_ptr, info_ptr);

    out.w = static_cast<int>(png_get_image_width(png_ptr,  info_ptr));
    out.h = static_cast<int>(png_get_image_height(png_ptr, info_ptr));

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
    for (int i = 0; i < out.h; ++i)
        row_ptrs[i] = reinterpret_cast<png_bytep>(&out.data[i * row_stride]);

    png_read_image(png_ptr, row_ptrs);
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    delete[] row_ptrs;

    return out;
}

// ── readAssetFile ──────────────────────────────────────────────────────────

BinaryBlob AppPlatform_wasm::readAssetFile(const std::string& filename) {
    std::string full_path = asset_base_path_ + filename;
    FILE* fp = std::fopen(full_path.c_str(), "rb");
    if (!fp) return BinaryBlob();

    long size = getRemainingFileSize(fp);
    if (size <= 0) { std::fclose(fp); return BinaryBlob(); }

    unsigned char* buf = new unsigned char[size];
    std::fread(buf, 1, static_cast<size_t>(size), fp);
    std::fclose(fp);

    return BinaryBlob(buf, static_cast<unsigned int>(size));
}

// ── getDateString ──────────────────────────────────────────────────────────

std::string AppPlatform_wasm::getDateString(int s) {
    time_t t = static_cast<time_t>(s);
    struct tm* tm_info = std::localtime(&t);
    char buf[64];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tm_info);
    return std::string(buf);
}

// ── World-name dialog ─────────────────────────────────────────────────────
// Immediately auto-confirm with the default world name "World".
// The game polls getUserInputStatus() each update; returning 1 on the first
// poll is sufficient — the game proceeds to world creation.

void AppPlatform_wasm::createUserInput() {
    text_input_buffer_ = "World";
    text_input_status_ = 1;  // immediately confirmed
    LOGI("[WASM] Auto-confirming world name: '%s'\n",
         text_input_buffer_.c_str());
}

int AppPlatform_wasm::getUserInputStatus() {
    return text_input_status_;
}

StringVector AppPlatform_wasm::getUserInput() {
    StringVector sv;
    sv.push_back(text_input_buffer_);
    return sv;
}
