#include "burymetal.h"
#include "bm_types.h"
#include "bm_backend.h"
#include "bm_utils.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// ----------------------------------------
// AMD Backend: создание и уничтожение устройства
// ----------------------------------------
BMDevice* bm_backend_create_device(BMComputeTarget type) {
    BMDevice* dev = (BMDevice*)malloc(sizeof(BMDevice));
    if (!dev) {
        bm_set_last_error("[AMD] Ошибка: не удалось выделить память для устройства");
        return NULL;
    }

    dev->type = BM_AMD;
    dev->handle = NULL;  // заглушка: здесь мог бы быть дескриптор ROCm
    snprintf(dev->name, sizeof(dev->name), "AMD GPU");
    dev->id = 0;

    bm_log_info("[AMD] Устройство создано: %s (id=%d)", dev->name, dev->id);
    return dev;
}

void bm_backend_destroy_device(BMDevice* device) {
    if (device) {
        bm_log_info("[AMD] Устройство уничтожено: %s (id=%d)", device->name, device->id);
        free(device);
    }
}

// ----------------------------------------
// AMD Backend: буферы GPU
// ----------------------------------------
BMBuffer* bm_backend_alloc_buffer(BMDevice* device, size_t size) {
    if (!device || size == 0) {
        bm_set_last_error("[AMD] alloc_buffer: некорректные аргументы");
        return NULL;
    }

    BMBuffer* buf = (BMBuffer*)malloc(sizeof(BMBuffer));
    if (!buf) {
        bm_set_last_error("[AMD] Ошибка выделения памяти под BMBuffer");
        return NULL;
    }

    buf->device = device;
    buf->size = size;
    buf->gpu_ptr = malloc(size); // заглушка вместо hipMalloc
    if (!buf->gpu_ptr) {
        free(buf);
        bm_set_last_error("[AMD] Ошибка выделения памяти под gpu_ptr");
        return NULL;
    }

    bm_log_debug("[AMD] Выделен буфер %zu байт", size);
    return buf;
}

void bm_backend_free_buffer(BMBuffer* buffer) {
    if (buffer) {
        if (buffer->gpu_ptr) free(buffer->gpu_ptr);
        bm_log_debug("[AMD] Освобождён буфер %zu байт", buffer->size);
        free(buffer);
    }
}

// ----------------------------------------
// AMD Backend: загрузка и скачивание данных
// ----------------------------------------
BMStatus bm_backend_upload_data(BMBuffer* buffer, const void* data) {
    if (!buffer || !data || !buffer->gpu_ptr) {
        bm_set_last_error("[AMD] upload_data: некорректные аргументы");
        return BM_STATUS_ERROR;
    }
    memcpy(buffer->gpu_ptr, data, buffer->size);
    bm_log_debug("[AMD] Загружены данные в буфер (%zu байт)", buffer->size);
    return BM_STATUS_OK;
}

BMStatus bm_backend_download_data(BMBuffer* buffer, void* data) {
    if (!buffer || !data || !buffer->gpu_ptr) {
        bm_set_last_error("[AMD] download_data: некорректные аргументы");
        return BM_STATUS_ERROR;
    }
    memcpy(data, buffer->gpu_ptr, buffer->size);
    bm_log_debug("[AMD] Скачаны данные из буфера (%zu байт)", buffer->size);
    return BM_STATUS_OK;
}

// ----------------------------------------
// AMD Backend: ядра
// ----------------------------------------
BMKernel* bm_backend_load_kernel(BMDevice* device, const char* kernel_path) {
    if (!device || !kernel_path) {
        bm_set_last_error("[AMD] load_kernel: некорректные аргументы");
        return NULL;
    }

    BMKernel* kernel = (BMKernel*)malloc(sizeof(BMKernel));
    if (!kernel) {
        bm_set_last_error("[AMD] Ошибка выделения памяти под BMKernel");
        return NULL;
    }

    kernel->device = device;
    kernel->kernel_ptr = NULL; // заглушка
    kernel->cpu_func = NULL;   // можно назначить CPU fallback
    snprintf(kernel->name, sizeof(kernel->name), "%s", kernel_path);

    bm_log_info("[AMD] Загружен kernel: %s", kernel->name);
    return kernel;
}

BMStatus bm_backend_run_kernel(BMKernel* kernel, BMBuffer* buffer, size_t count) {
    if (!kernel || !buffer) {
        bm_set_last_error("[AMD] run_kernel: некорректные аргументы");
        return BM_STATUS_ERROR;
    }

    bm_log_info("[AMD] Запуск kernel '%s' на %zu элементов", kernel->name, count);

    // CPU fallback для тестов
    if (kernel->cpu_func && buffer->gpu_ptr) {
        kernel->cpu_func(buffer->gpu_ptr, count);
        bm_log_debug("[AMD] Kernel выполнен на CPU fallback");
    }

    // TODO: hipLaunchKernel или OpenCL
    return BM_STATUS_OK;
}

void bm_backend_destroy_kernel(BMKernel* kernel) {
    if (kernel) {
        bm_log_info("[AMD] Kernel уничтожен: %s", kernel->name);
        free(kernel);
    }
}

// ----------------------------------------
// AMD Backend: синхронизация и query
// ----------------------------------------
BMStatus bm_backend_sync(BMDevice* device) {
    if (!device) return BM_STATUS_ERROR;
    bm_log_debug("[AMD] Синхронизация устройства %s", device->name);
    // TODO: hipDeviceSynchronize()
    return BM_STATUS_OK;
}

BMStatus bm_backend_query_device(BMDevice* device, BMDeviceInfo* info) {
    if (!device || !info) return BM_STATUS_ERROR;

    info->type = BM_AMD;
    snprintf(info->name, sizeof(info->name), "AMD GPU");
    info->memory_total = 8192;   // пример: 8 GB
    info->memory_free  = 4096;   // пример: 4 GB
    info->compute_units = 64;    // пример: 64 CU

    bm_log_info("[AMD] Запрос информации об устройстве: %s", info->name);
    return BM_STATUS_OK;
}
