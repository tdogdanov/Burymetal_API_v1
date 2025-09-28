// bm_kernel.c
#include "burymetal.h"
#include "bm_backend.h"
#include "bm_utils.h"

#include <stdlib.h>
#include <string.h>

#define BM_KERNEL_NAME_MAX 64

// -----------------------------
// Регистрация ядра (CPU-функция)
// -----------------------------
BMKernel* bm_register_kernel(BMDevice* device, const char* name, BMKernelFunc func) {
    if (!device || !name || !func) {
        bm_set_last_error("bm_register_kernel: некорректные аргументы");
        return NULL;
    }

    BMKernel* kernel = (BMKernel*)malloc(sizeof(BMKernel));
    if (!kernel) {
        bm_set_last_error("bm_register_kernel: не удалось выделить память");
        return NULL;
    }

    memset(kernel, 0, sizeof(*kernel));
    kernel->device = device;
    kernel->cpu_func = func;
    kernel->backend_kernel = NULL;
    strncpy(kernel->name, name, BM_KERNEL_NAME_MAX - 1);
    kernel->name[BM_KERNEL_NAME_MAX - 1] = '\0';

    bm_log(BM_LOG_INFO, "CPU kernel зарегистрировано: %s", kernel->name);
    return kernel;
}

// -----------------------------
// Загрузка ядра из backend (GPU)
// -----------------------------
BMKernel* bm_load_kernel_from_backend(BMDevice* device, const char* path) {
    if (!device || !path) {
        bm_set_last_error("bm_load_kernel_from_backend: некорректные аргументы");
        return NULL;
    }

    BMKernel* kernel = bm_backend_load_kernel(device, path);
    if (!kernel) {
        // bm_backend_load_kernel должен установить last_error
        return NULL;
    }

    bm_log(BM_LOG_INFO, "Backend kernel загружено: %s", kernel->name);
    return kernel;
}

// -----------------------------
// Унифицированный запуск ядра
// -----------------------------
BMResult bm_launch_kernel(BMKernel* kernel, BMBuffer* buf, size_t count) {
    if (!kernel || !buf || !buf->data) {
        bm_set_last_error("bm_launch_kernel: некорректные аргументы или буфер не инициализирован");
        return BM_ERROR_INVALID_ARG;
    }

    // CPU режим
    if (kernel->device->type == BM_CPU) {
        if (!kernel->cpu_func) {
            bm_set_last_error("bm_launch_kernel: CPU-ядро не задано");
            return BM_ERROR_INTERNAL;
        }
        kernel->cpu_func(buf->data, count);
        bm_log(BM_LOG_INFO, "CPU kernel %s выполнено на %zu элементов", kernel->name, count);
        return BM_OK;
    }

    // GPU / Backend режим
    if (!kernel->backend_kernel) {
        bm_set_last_error("bm_launch_kernel: backend ядро не загружено");
        return BM_ERROR_INVALID_ARG;
    }

    BMResult res = bm_backend_launch_kernel(kernel, buf, count);
    if (res != BM_OK) {
        // bm_backend_launch_kernel должен установить last_error
        bm_set_last_error("bm_launch_kernel: ошибка backend при запуске ядра");
        return res;
    }

    bm_log(BM_LOG_INFO, "Backend kernel %s выполнено на %zu элементов", kernel->name, count);
    return BM_OK;
}

// -----------------------------
// Удаление ядра
// -----------------------------
void bm_unregister_kernel(BMKernel* kernel) {
    if (!kernel) return;

    if (kernel->backend_kernel) {
        bm_backend_destroy_kernel(kernel);
    }

    bm_log(BM_LOG_INFO, "Ядро уничтожено: %s", kernel->name);
    free(kernel);
}
