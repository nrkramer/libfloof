#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#include "floof/floof.h"
#include "sound_registry.h"

#include <cstdlib>
#include <cstring>
#include <random>

// ---------------------------------------------------------------------------
// Custom VFS -- serves embedded byte arrays to miniaudio's resource manager
// so that ma_engine_play_sound() "just works" with sound names as paths.
// ---------------------------------------------------------------------------

struct floof_vfs_file_handle {
    const unsigned char* data;
    size_t               size;
    size_t               cursor;
};

static ma_result floof_vfs_open(ma_vfs* pVFS, const char* pFilePath,
                                ma_uint32 openMode, ma_vfs_file* pFile)
{
    (void)pVFS;
    if (openMode & MA_OPEN_MODE_WRITE) return MA_ACCESS_DENIED;

    for (size_t i = 0; i < floof_embedded_sound_count; i++) {
        if (std::strcmp(floof_embedded_sounds[i].name, pFilePath) == 0) {
            auto* h = static_cast<floof_vfs_file_handle*>(
                          std::malloc(sizeof(floof_vfs_file_handle)));
            if (!h) return MA_OUT_OF_MEMORY;
            h->data   = floof_embedded_sounds[i].data;
            h->size   = floof_embedded_sounds[i].size;
            h->cursor = 0;
            *pFile = h;
            return MA_SUCCESS;
        }
    }
    return MA_DOES_NOT_EXIST;
}

static ma_result floof_vfs_close(ma_vfs* pVFS, ma_vfs_file file)
{
    (void)pVFS;
    std::free(file);
    return MA_SUCCESS;
}

static ma_result floof_vfs_read(ma_vfs* pVFS, ma_vfs_file file,
                                void* pDst, size_t sizeInBytes,
                                size_t* pBytesRead)
{
    (void)pVFS;
    auto* h = static_cast<floof_vfs_file_handle*>(file);
    size_t remaining = h->size - h->cursor;
    size_t toRead    = (sizeInBytes < remaining) ? sizeInBytes : remaining;

    if (toRead == 0) {
        if (pBytesRead) *pBytesRead = 0;
        return MA_AT_END;
    }

    std::memcpy(pDst, h->data + h->cursor, toRead);
    h->cursor += toRead;
    if (pBytesRead) *pBytesRead = toRead;
    return (toRead == sizeInBytes) ? MA_SUCCESS : MA_AT_END;
}

static ma_result floof_vfs_seek(ma_vfs* pVFS, ma_vfs_file file,
                                ma_int64 offset, ma_seek_origin origin)
{
    (void)pVFS;
    auto* h = static_cast<floof_vfs_file_handle*>(file);
    ma_int64 newCursor;

    switch (origin) {
        case ma_seek_origin_start:   newCursor = offset; break;
        case ma_seek_origin_current: newCursor = static_cast<ma_int64>(h->cursor) + offset; break;
        case ma_seek_origin_end:     newCursor = static_cast<ma_int64>(h->size)   + offset; break;
        default: return MA_INVALID_ARGS;
    }

    if (newCursor < 0 || static_cast<size_t>(newCursor) > h->size)
        return MA_BAD_SEEK;

    h->cursor = static_cast<size_t>(newCursor);
    return MA_SUCCESS;
}

static ma_result floof_vfs_tell(ma_vfs* pVFS, ma_vfs_file file,
                                ma_int64* pCursor)
{
    (void)pVFS;
    auto* h = static_cast<floof_vfs_file_handle*>(file);
    *pCursor = static_cast<ma_int64>(h->cursor);
    return MA_SUCCESS;
}

static ma_result floof_vfs_info(ma_vfs* pVFS, ma_vfs_file file,
                                ma_file_info* pInfo)
{
    (void)pVFS;
    auto* h = static_cast<floof_vfs_file_handle*>(file);
    pInfo->sizeInBytes = h->size;
    return MA_SUCCESS;
}

// The first member MUST be ma_vfs_callbacks so that miniaudio can cast
// a floof_vfs* to ma_vfs_callbacks* transparently.
struct floof_vfs {
    ma_vfs_callbacks cb;
};

static floof_vfs make_embedded_vfs()
{
    floof_vfs vfs{};
    vfs.cb.onOpen  = floof_vfs_open;
    vfs.cb.onOpenW = nullptr;
    vfs.cb.onClose = floof_vfs_close;
    vfs.cb.onRead  = floof_vfs_read;
    vfs.cb.onWrite = nullptr;
    vfs.cb.onSeek  = floof_vfs_seek;
    vfs.cb.onTell  = floof_vfs_tell;
    vfs.cb.onInfo  = floof_vfs_info;
    return vfs;
}

// ---------------------------------------------------------------------------
// Context
// ---------------------------------------------------------------------------

struct floof_context {
    floof_vfs    vfs;
    ma_engine    engine;
    std::mt19937 rng;
};

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

extern "C" {

int floof_init(floof_context** ctx)
{
    if (!ctx) return -1;

    *ctx = new (std::nothrow) floof_context();
    if (!*ctx) return -1;

    (*ctx)->vfs = make_embedded_vfs();
    (*ctx)->rng.seed(std::random_device{}());

    ma_engine_config config = ma_engine_config_init();
    config.pResourceManagerVFS = &(*ctx)->vfs;

    if (ma_engine_init(&config, &(*ctx)->engine) != MA_SUCCESS) {
        delete *ctx;
        *ctx = nullptr;
        return -1;
    }

    return 0;
}

void floof_shutdown(floof_context* ctx)
{
    if (!ctx) return;
    ma_engine_uninit(&ctx->engine);
    delete ctx;
}

size_t floof_sound_count(void)
{
    return floof_embedded_sound_count;
}

const char* floof_sound_name(size_t index)
{
    if (index >= floof_embedded_sound_count) return nullptr;
    return floof_embedded_sounds[index].name;
}

int floof_play(floof_context* ctx, const char* name)
{
    if (!ctx || !name) return -1;
    return (ma_engine_play_sound(&ctx->engine, name, nullptr) == MA_SUCCESS)
               ? 0 : -1;
}

int floof_play_random(floof_context* ctx)
{
    if (!ctx || floof_embedded_sound_count == 0) return -1;

    std::uniform_int_distribution<size_t> dist(0, floof_embedded_sound_count - 1);
    size_t index = dist(ctx->rng);
    return floof_play(ctx, floof_embedded_sounds[index].name);
}

} // extern "C"
