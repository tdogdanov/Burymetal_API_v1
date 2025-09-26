// test_main.c
#include "burymetal.h"
#include <stdio.h>
#include <stdlib.h>

#define N 8

// --- Пример ядра: удвоение элементов ---
void double_kernel(void* data, size_t count) {
    float* arr = (float*)data;
    for (size_t i = 0; i < count; i++) arr[i] *= 2.0f;
}

// --- RAII-обёртки для автоматической очистки ---
typedef struct { BMDevice* device; } BMDeviceGuard;
typedef struct { BMBuffer* buffer; } BMBufferGuard;
typedef struct { BMKernel* kernel; } BMKernelGuard;

static void free_device(BMDeviceGuard* g) {
    if (g && g->device) {
        bm_destroy_device(g->device);
        g->device = NULL;
    }
}

static void free_buffer(BMBufferGuard* g) {
    if (g && g->buffer) {
        bm_free_buffer(g->buffer);
        g->buffer = NULL;
    }
}

static void free_kernel(BMKernelGuard* g) {
    if (g && g->kernel) {
        /* Используем unregister (если у тебя другое имя — замените на реализацию) */
        bm_unregister_kernel(g->kernel);
        g->kernel = NULL;
    }
}

int main(void) {
    bm_log_set_level(BM_LOG_LEVEL_DEBUG);
    bm_log_info("=== Burymetal RAII Test ===");

    // --- Создание устройства (автовыбор) ---
    BMDeviceGuard dev_guard = { .device = bm_create_device(BM_AUTO) };
    if (!dev_guard.device) {
        bm_log_error("Ошибка создания устройства: %s", bm_get_last_error());
        return 1;
    }

    // Информация об устройстве (опционально)
    BMDeviceInfo info;
    if (bm_query_device(dev_guard.device, &info) == BM_SUCCESS) {
        bm_log_info("Используемое устройство: %s, CU=%d, RAM=%zu",
                    info.name, info.compute_units, info.memory_size);
    } else {
        bm_log_warn("Не удалось получить информацию об устройстве");
    }

    // --- Создание буфера ---
    float input[N]  = {1,2,3,4,5,6,7,8};
    float output[N] = {0};
    BMBufferGuard buf_guard = { .buffer = bm_alloc_buffer(dev_guard.device, sizeof(input)) };
    if (!buf_guard.buffer) {
        bm_log_error("Ошибка выделения буфера: %s", bm_get_last_error());
        free_device(&dev_guard);
        return 1;
    }

    // --- Загрузка данных в буфер ---
    if (bm_upload_data(buf_guard.buffer, input) != BM_SUCCESS) {
        bm_log_error("Ошибка загрузки данных: %s", bm_get_last_error());
        free_buffer(&buf_guard);
        free_device(&dev_guard);
        return 1;
    }

    // --- Регистрация ядра ---
    BMKernelGuard kernel_guard = { .kernel = bm_register_kernel(dev_guard.device, "double_kernel", double_kernel) };
    if (!kernel_guard.kernel) {
        bm_log_error("Ошибка регистрации ядра: %s", bm_get_last_error());
        free_buffer(&buf_guard);
        free_device(&dev_guard);
        return 1;
    }

    // --- Запуск ядра ---
    if (bm_launch_kernel(kernel_guard.kernel, buf_guard.buffer, N) != BM_SUCCESS) {
        bm_log_error("Ошибка запуска ядра: %s", bm_get_last_error());
        free_kernel(&kernel_guard);
        free_buffer(&buf_guard);
        free_device(&dev_guard);
        return 1;
    }

    // --- Скачивание результата ---
    if (bm_download_data(buf_guard.buffer, output) != BM_SUCCESS) {
        bm_log_error("Ошибка скачивания данных: %s", bm_get_last_error());
        free_kernel(&kernel_guard);
        free_buffer(&buf_guard);
        free_device(&dev_guard);
        return 1;
    }

    // --- Проверка и вывод результата ---
    int ok = 1;
    for (size_t i = 0; i < N; i++) {
        float expect = input[i] * 2.0f;
        printf("in=%.1f -> out=%.1f (expect=%.1f)\n", input[i], output[i], expect);
        if (output[i] != expect) ok = 0;
    }
    bm_log_info("Результат: %s", ok ? "OK ✅" : "FAIL ❌");

    // --- Очистка ресурсов (RAII) ---
    free_kernel(&kernel_guard);
    free_buffer(&buf_guard);
    free_device(&dev_guard);

    return ok ? 0 : 2;
}
