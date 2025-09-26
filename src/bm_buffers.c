#include "burymetal.h"
#include "bm_utils.h"
#include <stdlib.h>
#include <string.h>

// ----------------------------------------
// Выделение буфера на устройстве
// ----------------------------------------
BMBuffer* bm_alloc_buffer(BMDevice* device, size_t size) {
    if (!device || size == 0) {
        bm_set_last_error("[BM] bm_alloc_buffer: некорректные аргументы");
        return NULL;
    }

    BMBuffer* buf = (BMBuffer*)malloc(sizeof(BMBuffer));
    if (!buf) {
        bm_set_last_error("[BM] bm_alloc_buffer: не удалось выделить память для BMBuffer");
        return NULL;
    }

    buf->device = device;
    buf->size = size;

    // backend выделяет память (CPU/GPU)
    buf->gpu_ptr = bm_backend_alloc_buffer(device, size);
    if (!buf->gpu_ptr) {
        free(buf);
        bm_set_last_error("[BM] bm_alloc_buffer: не удалось выделить память на backend");
        return NULL;
    }

    bm_log_info("[BM] Буфер выделен: %zu байт на устройстве %s", size, device->name);
    return buf;
}

// ----------------------------------------
// Освобождение буфера
// ----------------------------------------
void bm_free_buffer(BMBuffer* buf) {
    if (!buf) {
        bm_log_warn("[BM] bm_free_buffer: buf == NULL");
        return;
    }

    if (buf->gpu_ptr) {
        bm_backend_free_buffer(buf);
        bm_log_info("[BM] Буфер освобождён: %zu байт на устройстве %s", buf->size, buf->device->name);
    }

    free(buf);
}

// ----------------------------------------
// Запись данных в буфер
// ----------------------------------------
void bm_write_buffer(BMBuffer* buf, const void* data, size_t size, size_t offset) {
    if (!buf || !data) {
        bm_set_last_error("[BM] bm_write_buffer: некорректные аргументы");
        return;
    }
    if (offset + size > buf->size) {
        bm_set_last_error("[BM] bm_write_buffer: выход за границы буфера");
        return;
    }

    bm_backend_upload_data(buf, ((char*)data) + offset);
    bm_log_debug("[BM] Данные записаны в буфер: %zu байт, offset=%zu", size, offset);
}

// ----------------------------------------
// Чтение данных из буфера
// ----------------------------------------
void bm_read_buffer(BMBuffer* buf, void* data, size_t size, size_t offset) {
    if (!buf || !data) {
        bm_set_last_error("[BM] bm_read_buffer: некорректные аргументы");
        return;
    }
    if (offset + size > buf->size) {
        bm_set_last_error("[BM] bm_read_buffer: выход за границы буфера");
        return;
    }

    bm_backend_download_data(buf, ((char*)data) + offset);
    bm_log_debug("[BM] Данные прочитаны из буфера: %zu байт, offset=%zu", size, offset);
}
