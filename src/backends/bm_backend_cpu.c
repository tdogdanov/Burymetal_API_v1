#include "burymetal.h"
#include "bm_types.h"
#include "bm_backend.h"
#include "bm_utils.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// ----------------------------------------
// CPU Backend: создание и уничтожение устройства
// ----------------------------------------
BMDevice* bm_backend_create_device(BMComputeTarget type) {
    (void)type;

    BMDevice* dev = (BMDevice*)malloc(sizeof(BMDevice));
    if (!dev) {
        bm_set_last_error("[CPU] Ошибка выделения памяти для BMDevice");
        return NULL;
    }

    dev->type = BM_CPU;
    dev->handle = NULL;
    snprintf(dev->name, sizeof(dev->name), "CPU Device");
    dev->id = 0;

    bm_log_info("[CPU] Устройство создано: %s", dev->name);
    return dev;
}

void bm_backend_destroy_device(BMDevice* device) {
    if (device) {
        bm_log_info("[CPU] Устройство уничтожено: %s", device->name);
        free(device);
    }
}

// ----------------------------------------
// CPU Backend: буферы памяти
// ----------------------------------------
BMBuffer* bm_backend_alloc_buffer(BMDevice* device, size_t size) {
    if (!device || size == 0) {
        bm_set_last_error("[CPU] alloc_buffer: некорректные аргументы");
        return NULL;
    }

    BMBuffer* buf = (BMBuffer*)malloc(sizeof(BMBuffer));
    if (!buf) {
        bm_set_last_error("[CPU] Ошибка выделения памяти для BMBuffer");
        return NULL;
    }

    buf->device = device;
    buf->size = size;
    buf->gpu_ptr = malloc(size);
    if (!buf->gpu_ptr) {
        free(buf);
        bm_set_last_error("[CPU] Ошибка выделения памяти для gpu_ptr");
        return NULL;
    }

    bm_log_debug("[CPU] Выделен буфер %zu байт", size);
    return buf;
}

void bm_backend_free_buffer(BMBuffer* buffer) {
    if (buffer) {
        if (buffer->gpu_ptr) free(buffer->gpu_ptr);
        bm_log_debug("[CPU] Освобождён буфер %zu байт", buffer->size);
        free(buffer);
    }
}

// ----------------------------------------
// CPU Backend: загрузка и скачивание данных
// ----------------------------------------
BMStatus bm_backend_upload_data(BMBuffer* buffer, const void* data) {
    if (!buffer || !data || !buffer->gpu_ptr) {
        bm_set_last_error("[CPU] upload_data: некорректные аргументы");
        return BM_STATUS_ERROR;
    }
    memcpy(buffer->gpu_ptr, data, buffer->size);
    bm_log_debug("[CPU] Загружены данные в буфер (%zu байт)", buffer->size);
    return BM_STATUS_OK;
}

BMStatus bm_backend_download_data(BMBuffer* buffer, void* data) {
    if (!buffer || !data || !buffer->gpu_ptr) {
        bm_set_last_error("[CPU] download_data: некорректные аргументы");
        return BM_STATUS_ERROR;
    }
    memcpy(data, buffer->gpu_ptr, buffer->size);
    bm_log_debug("[CPU] Скачаны данные из буфера (%zu байт)", buffer->size);
    return BM_STATUS_OK;
}

// ----------------------------------------
// CPU Backend: ядра
// ----------------------------------------
BMKernel* bm_backend_load_kernel(BMDevice* device, const char* kernel_path) {
    (void)kernel_path;

    BMKernel* kernel = (BMKernel*)malloc(sizeof(BMKernel));
    if (!kernel) {
        bm_set_last_error("[CPU] Ошибка выделения памяти для BMKernel");
        return NULL;
    }

    kernel->device = device;
    kernel->kernel_ptr = NULL;
    kernel->cpu_func = NULL; // можно назначить функцию для CPU fallback
    snprintf(kernel->name, sizeof(kernel->name), "CPU Kernel");

    bm_log_info("[CPU] Kernel загружен: %s", kernel->name);
    return kernel;
}

BMStatus bm_backend_run_kernel(BMKernel* kernel, BMBuffer* buffer, size_t count) {
    if (!kernel || !buffer || !buffer->gpu_ptr) {
        bm_set_last_error("[CPU] run_kernel: некорректные аргументы");
        return BM_STATUS_ERROR;
    }

    bm_log_info("[CPU] Запуск kernel '%s' на %zu элементов", kernel->name, count);

    // CPU fallback
    if (kernel->cpu_func) {
        kernel->cpu_func(buffer->gpu_ptr, count);
        bm_log_debug("[CPU] Kernel выполнен на CPU fallback");
    }

    return BM_STATUS_OK;
}

void bm_backend_destroy_kernel(BMKernel* kernel) {
    if (kernel) {
        bm_log_info("[CPU] Kernel уничтожен: %s", kernel->name);
        free(kernel);
    }
}

// ----------------------------------------
// CPU Backend: синхронизация и query
// ----------------------------------------
BMStatus bm_backend_sync(BMDevice* device) {
    if (!device) return BM_STATUS_ERROR;
    bm_log_debug("[CPU] sync (no-op)");
    return BM_STATUS_OK;
}

BMStatus bm_backend_query_device(BMDevice* device, BMDeviceInfo* info) {
    if (!device || !info) return BM_STATUS_ERROR;

    info->type = BM_CPU;
    snprintf(info->name, sizeof(info->name), "CPU Device");
    info->memory_total = 16384; // 16 GB
    info->memory_free  = 8192;  // 8 GB
    info->compute_units = 8;    // 8 потоков

    bm_log_info("[CPU] Запрос информации об устройстве: %s", info->name);
    return BM_STATUS_OK;
}
