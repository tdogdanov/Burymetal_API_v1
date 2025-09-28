#include "burymetal.h"
#include "bm_types.h"
#include "bm_backend.h"
#include "bm_utils.h"

#include <stdlib.h>
#include <string.h>

// ----------------------------------------
// Intel Backend: создание и уничтожение устройства
// ----------------------------------------
BMDevice* bm_backend_create_device(BMComputeTarget type) {
    (void)type;

    BMDevice* dev = (BMDevice*)malloc(sizeof(BMDevice));
    if (!dev) {
        bm_set_last_error("[Intel] Ошибка выделения памяти для BMDevice");
        return NULL;
    }

    dev->type = BM_INTEL;
    dev->handle = NULL;
    snprintf(dev->name, sizeof(dev->name), "Intel GPU");
    dev->id = 0;

    bm_log_info("[Intel] Устройство создано: %s", dev->name);
    return dev;
}

void bm_backend_destroy_device(BMDevice* device) {
    if (device) {
        bm_log_info("[Intel] Устройство уничтожено: %s", device->name);
        free(device);
    }
}

// ----------------------------------------
// Intel Backend: буферы GPU
// ----------------------------------------
BMBuffer* bm_backend_alloc_buffer(BMDevice* device, size_t size) {
    if (!device || size == 0) {
        bm_set_last_error("[Intel] alloc_buffer: некорректные аргументы");
        return NULL;
    }

    BMBuffer* buf = (BMBuffer*)malloc(sizeof(BMBuffer));
    if (!buf) {
        bm_set_last_error("[Intel] Ошибка выделения памяти для BMBuffer");
        return NULL;
    }

    buf->device = device;
    buf->size = size;
    buf->gpu_ptr = malloc(size); // заглушка вместо oneAPI / Level Zero
    if (!buf->gpu_ptr) {
        free(buf);
        bm_set_last_error("[Intel] Ошибка выделения памяти для gpu_ptr");
        return NULL;
    }

    bm_log_debug("[Intel] Выделен буфер %zu байт", size);
    return buf;
}

void bm_backend_free_buffer(BMBuffer* buffer) {
    if (!buffer) return;
    if (buffer->gpu_ptr) free(buffer->gpu_ptr);
    bm_log_debug("[Intel] Освобождён буфер %zu байт", buffer->size);
    free(buffer);
}

// ----------------------------------------
// Intel Backend: загрузка и скачивание данных
// ----------------------------------------
BMStatus bm_backend_upload_data(BMBuffer* buffer, const void* data) {
    if (!buffer || !data || !buffer->gpu_ptr) {
        bm_set_last_error("[Intel] upload_data: некорректные аргументы");
        return BM_STATUS_ERROR;
    }
    memcpy(buffer->gpu_ptr, data, buffer->size);
    bm_log_debug("[Intel] Загружены данные в буфер (%zu байт)", buffer->size);
    return BM_STATUS_OK;
}

BMStatus bm_backend_download_data(BMBuffer* buffer, void* data) {
    if (!buffer || !data || !buffer->gpu_ptr) {
        bm_set_last_error("[Intel] download_data: некорректные аргументы");
        return BM_STATUS_ERROR;
    }
    memcpy(data, buffer->gpu_ptr, buffer->size);
    bm_log_debug("[Intel] Скачаны данные из буфера (%zu байт)", buffer->size);
    return BM_STATUS_OK;
}

// ----------------------------------------
// Intel Backend: ядра
// ----------------------------------------
BMKernel* bm_backend_load_kernel(BMDevice* device, const char* kernel_path) {
    if (!device || !kernel_path) {
        bm_set_last_error("[Intel] load_kernel: некорректные аргументы");
        return NULL;
    }

    BMKernel* kernel = (BMKernel*)malloc(sizeof(BMKernel));
    if (!kernel) {
        bm_set_last_error("[Intel] Ошибка выделения памяти для BMKernel");
        return NULL;
    }

    kernel->device = device;
    kernel->kernel_ptr = NULL; // заглушка
    kernel->cpu_func = NULL;   // CPU fallback при необходимости
    snprintf(kernel->name, sizeof(kernel->name), "%s", kernel_path);

    bm_log_info("[Intel] Kernel загружен: %s", kernel->name);
    return kernel;
}

BMStatus bm_backend_run_kernel(BMKernel* kernel, BMBuffer* buffer, size_t count) {
    if (!kernel || !buffer || !buffer->gpu_ptr) {
        bm_set_last_error("[Intel] run_kernel: некорректные аргументы");
        return BM_STATUS_ERROR;
    }

    bm_log_info("[Intel] Запуск kernel '%s' на %zu элементов", kernel->name, count);

    // CPU fallback для тестирования
    if (kernel->cpu_func) {
        kernel->cpu_func(buffer->gpu_ptr, count);
        bm_log_debug("[Intel] Kernel выполнен на CPU fallback");
    }

    // TODO: вызов через oneAPI / Level Zero
    return BM_STATUS_OK;
}

void bm_backend_destroy_kernel(BMKernel* kernel) {
    if (kernel) {
        bm_log_info("[Intel] Kernel уничтожен: %s", kernel->name);
        free(kernel);
    }
}

// ----------------------------------------
// Intel Backend: синхронизация и query
// ----------------------------------------
BMStatus bm_backend_sync(BMDevice* device) {
    if (!device) return BM_STATUS_ERROR;
    bm_log_debug("[Intel] sync (no-op)");
    return BM_STATUS_OK;
}

BMStatus bm_backend_query_device(BMDevice* device, BMDeviceInfo* info) {
    if (!device || !info) return BM_STATUS_ERROR;

    info->type = BM_INTEL;
    snprintf(info->name, sizeof(info->name), "Intel GPU");
    info->memory_total = 16384; // 16 GB
    info->memory_free  = 8192;  // 8 GB
    info->compute_units = 128;  // пример: 128 CU

    bm_log_info("[Intel] Запрос информации об устройстве: %s", info->name);
    return BM_STATUS_OK;
}
