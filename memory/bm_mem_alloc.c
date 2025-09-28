#include "bm_mem_alloc.h"
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

// --- CPU память ---
BMResult bm_cpu_alloc(size_t size, void** out_ptr) {
    if (!out_ptr || size == 0) {
        bm_set_last_error("bm_cpu_alloc: invalid arguments");
        return BM_ERROR;
    }

    void* ptr = malloc(size);
    if (!ptr) {
        bm_set_last_error("bm_cpu_alloc: out of memory");
        return BM_ERROR;
    }

    memset(ptr, 0, size);
    *out_ptr = ptr;
    return BM_SUCCESS;
}

BMResult bm_cpu_free(void* ptr) {
    if (!ptr) return BM_ERROR;
    free(ptr);
    return BM_SUCCESS;
}

// --- GPU память (stub для CPU-only, интегрировать backend позже) ---
BMResult bm_gpu_alloc(BMDevice* device, size_t size, void** out_ptr) {
    if (!device || !out_ptr || size == 0) {
        bm_set_last_error("bm_gpu_alloc: invalid arguments");
        return BM_ERROR;
    }

    if (device->type == BM_CPU) {
        // для CPU выделяем обычный RAM
        return bm_cpu_alloc(size, out_ptr);
    }

    // TODO: заменить на вызовы CUDA/ROCm/OneAPI
    bm_set_last_error("bm_gpu_alloc: GPU backend not implemented yet");
    *out_ptr = NULL;
    return BM_ERROR;
}

BMResult bm_gpu_free(BMDevice* device, void* ptr) {
    if (!device || !ptr) return BM_ERROR;

    if (device->type == BM_CPU) {
        return bm_cpu_free(ptr);
    }

    // TODO: GPU free
    bm_set_last_error("bm_gpu_free: GPU backend not implemented yet");
    return BM_ERROR;
}

// --- Копирование данных ---
BMResult bm_memcpy_host_to_device(BMBuffer* buffer, const void* src, size_t size) {
    if (!buffer || !src || size > buffer->size) {
        bm_set_last_error("bm_memcpy_host_to_device: invalid arguments");
        return BM_ERROR;
    }

    if (buffer->device->type == BM_CPU) {
        memcpy(buffer->data, src, size);
        return BM_SUCCESS;
    }

    // TODO: GPU upload через backend
    bm_set_last_error("bm_memcpy_host_to_device: GPU upload not implemented");
    return BM_ERROR;
}

BMResult bm_memcpy_device_to_host(BMBuffer* buffer, void* dst, size_t size) {
    if (!buffer || !dst || size > buffer->size) {
        bm_set_last_error("bm_memcpy_device_to_host: invalid arguments");
        return BM_ERROR;
    }

    if (buffer->device->type == BM_CPU) {
        memcpy(dst, buffer->data, size);
        return BM_SUCCESS;
    }

    // TODO: GPU download через backend
    bm_set_last_error("bm_memcpy_device_to_host: GPU download not implemented");
    return BM_ERROR;
}

BMResult bm_memcpy_device_to_device(BMBuffer* dst, BMBuffer* src, size_t size) {
    if (!dst || !src || size > dst->size || size > src->size) {
        bm_set_last_error("bm_memcpy_device_to_device: invalid arguments");
        return BM_ERROR;
    }

    if (dst->device->type == BM_CPU && src->device->type == BM_CPU) {
        memcpy(dst->data, src->data, size);
        return BM_SUCCESS;
    }

    // TODO: GPU ↔ GPU copy через backend
    bm_set_last_error("bm_memcpy_device_to_device: GPU copy not implemented");
    return BM_ERROR;
}
