// bm_kernel.c
#include "burymetal.h"
#include "bm_backend.h"
#include "bm_utils.h"

#include <stdlib.h>
#include <string.h>

#define BM_KERNEL_NAME_MAX 64

// Регистрация ядра (CPU-функция)
BMKernel* bm_register_kernel(BMDevice* device, const char* name, BMKernelFunc func) {
    if (!device || !name || !func) {
        bm_set_last_error("[BM] bm_register_kernel: некорректные аргументы");
        return NULL;
    }

    BMKernel* kernel = (BMKernel*)malloc(sizeof(BMKernel));
    if (!kernel) {
        bm_set_last_error("[BM] bm_register_kernel: не удалось выделить память");
        return NULL;
    }

    memset(kernel, 0, sizeof(*kernel));
    kernel->device = device;
    kernel->cpu_func = func;           // явная CPU-функция
    kernel->kernel_ptr = NULL;         // backend-поле (если понадобится)
    kernel->backend_kernel = NULL;
    strncpy(kernel->name, name, sizeof(kernel->name) - 1);
    kernel->name[sizeof(kernel->name) - 1] = '\0';

    bm_log_info("[BM] Ядро зарегистрировано: %s", kernel->name);
    return kernel;
}

// Загрузка ядра из файла/пакета для backend (опционально)
BMKernel* bm_load_kernel_from_backend(BMDevice* device, const char* path) {
    if (!device || !path) {
        bm_set_last_error("[BM] bm_load_kernel_from_backend: некорректные аргументы");
        return NULL;
    }

    // Делегируем backend'у загрузку (например CUDA module / ROCm kernel)
    BMKernel* kernel = bm_backend_load_kernel(device, path);
    if (!kernel) {
        // bm_backend_load_kernel должен установить last_error
        return NULL;
    }

    // backend должен заполнить kernel->kernel_ptr / kernel->backend_kernel / name
    bm_log_info("[BM] Backend kernel loaded: %s", kernel->name);
    return kernel;
}

// Унифицированный запуск ядра
BMStatus bm_launch_kernel(BMKernel* kernel, BMBuffer* buf, size_t count) {
    if (!kernel || !buf) {
        bm_set_last_error("[BM] bm_launch_kernel: некорректные аргументы");
        return BM_ERROR;
    }

    // Поддержка двух режимов: CPU (локальная функция) и backend (GPU)
    if (kernel->device->type == BM_CPU) {
        if (!kernel->cpu_func) {
            bm_set_last_error("[BM] bm_launch_kernel: CPU-ядро не задано");
            return BM_ERROR;
        }

        // буферные данные — используем поле data (сделай единообразие в bm_types)
        if (!buf->data) {
            bm_set_last_error("[BM] bm_launch_kernel: буфер не инициализирован");
            return BM_ERROR;
        }

        kernel->cpu_func(buf->data, count);
        bm_log_info("[BM] CPU kernel %s выполнено на %zu элементов", kernel->name, count);
        return BM_SUCCESS;
    }

    // Не-CPU: делегируем backend
    if (!kernel->device) {
        bm_set_last_error("[BM] bm_launch_kernel: устройство не задано");
        return BM_ERROR;
    }

    // Проверяем, что backend умеет запускать ядро
    BMStatus st = bm_backend_launch_kernel(kernel, buf, count);
    if (st != BM_SUCCESS) {
        // bm_backend_launch_kernel должен установить last_error
        bm_set_last_error("[BM] bm_launch_kernel: ошибка backend при запуске ядра");
        return st;
    }

#ifdef BM_USE_CUDA
    // опционально: проверка CUDA-ошибок, если backend использует CUDA
    cudaError_t err = cudaGetLastError();
    if (err != cudaSuccess) {
        bm_set_last_error("[BM] bm_launch_kernel: CUDA ошибка: %s", cudaGetErrorString(err));
        return BM_ERROR;
    }
    cudaDeviceSynchronize();
#endif

    bm_log_info("[BM] Backend kernel %s выполнено на %zu элементов", kernel->name, count);
    return BM_SUCCESS;
}

// Удаление ядра (унифицированно)
void bm_unregister_kernel(BMKernel* kernel) {
    if (!kernel) return;

    // Если backend выделял ресурсы — освобождаем через backend API
    if (kernel->backend_kernel) {
        bm_backend_destroy_kernel(kernel);
    }

    bm_log_info("[BM] Ядро уничтожено: %s", kernel->name);
    free(kernel);
}
