// bm_buffer.c
#include "burymetal.h"
#include "bm_utils.h"
#include "bm_backend.h"

#include <stdlib.h>
#include <string.h>

// ----------------------------------------
// Выделение буфера
// ----------------------------------------
BMBuffer* bm_alloc_buffer(BMDevice* device, size_t size) {
    if (!device || size == 0) {
        bm_set_last_error("bm_alloc_buffer: некорректные аргументы");
        return NULL;
    }

    BMBuffer* buf = (BMBuffer*)malloc(sizeof(BMBuffer));
    if (!buf) {
        bm_set_last_error("bm_alloc_buffer: не удалось выделить память для BMBuffer");
        return NULL;
    }

    buf->device = device;
    buf->size = size;
    buf->backend_ptr = bm_backend_alloc_buffer(device, size);
    buf->backend = device->type;

    if (!buf->backend_ptr) {
        free(buf);
        bm_set_last_error("bm_alloc_buffer: backend не смог выделить память");
        return NULL;
    }

    bm_log(BM_LOG_INFO, "Буфер выделен: %zu байт на устройстве %s", size, device->name);
    return buf;
}

// ----------------------------------------
// Освобождение буфера
// ----------------------------------------
BMResult bm_free_buffer(BMBuffer* buf) {
    if (!buf) {
        bm_log(BM_LOG_WARN, "bm_free_buffer: buf == NULL");
        return BM_ERROR_INVALID_ARG;
    }

    if (buf->backend_ptr) {
        bm_backend_free_buffer(buf);
        bm_log(BM_LOG_INFO, "Буфер освобождён: %zu байт на устройстве %s", buf->size, buf->device->name);
    }

    free(buf);
    return BM_OK;
}

// ----------------------------------------
// Запись данных в буфер
// ----------------------------------------
BMResult bm_write_buffer(BMBuffer* buf, const void* data, size_t size, size_t offset) {
    if (!buf || !data) return BM_ERROR_INVALID_ARG;
    if (offset + size > buf->size) return BM_ERROR_INVALID_ARG;

    BMResult res = bm_backend_upload_data(buf, ((char*)data) + offset, size, offset);
    if (res != BM_OK) {
        bm_set_last_error("bm_write_buffer: ошибка при записи в backend");
        return res;
    }

    bm_log(BM_LOG_DEBUG, "Данные записаны в буфер: %zu байт, offset=%zu", size, offset);
    return BM_OK;
}

// ----------------------------------------
// Чтение данных из буфера
// ----------------------------------------
BMResult bm_read_buffer(BMBuffer* buf, void* data, size_t size, size_t offset) {
    if (!buf || !data) return BM_ERROR_INVALID_ARG;
    if (offset + size > buf->size) return BM_ERROR_INVALID_ARG;

    BMResult res = bm_backend_download_data(buf, ((char*)data) + offset, size, offset);
    if (res != BM_OK) {
        bm_set_last_error("bm_read_buffer: ошибка при чтении из backend");
        return res;
    }

    bm_log(BM_LOG_DEBUG, "Данные прочитаны из буфера: %zu байт, offset=%zu", size, offset);
    return BM_OK;
}

// ----------------------------------------
// Работа с массивом буферов (для ядра)
// ----------------------------------------
BMResult bm_write_buffers(BMBuffer** bufs, size_t num_bufs, const void** data, const size_t* sizes, const size_t* offsets) {
    if (!bufs || !data || !sizes || !offsets) return BM_ERROR_INVALID_ARG;
    for (size_t i = 0; i < num_bufs; ++i) {
        BMResult res = bm_write_buffer(bufs[i], data[i], sizes[i], offsets[i]);
        if (res != BM_OK) return res;
    }
    return BM_OK;
}

BMResult bm_read_buffers(BMBuffer** bufs, size_t num_bufs, void** data, const size_t* sizes, const size_t* offsets) {
    if (!bufs || !data || !sizes || !offsets) return BM_ERROR_INVALID_ARG;
    for (size_t i = 0; i < num_bufs; ++i) {
        BMResult res = bm_read_buffer(bufs[i], data[i], sizes[i], offsets[i]);
        if (res != BM_OK) return res;
    }
    return BM_OK;
}
