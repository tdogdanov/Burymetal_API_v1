#include "burymetal.h"
#include <stdio.h>
#include <stdlib.h>

#define N 4

// --- Пример ядра: удвоение каждого элемента ---
void double_kernel(void* data, size_t count) {
    float* arr = (float*)data;
    for (size_t i = 0; i < count; i++) {
        arr[i] *= 2.0f;
    }
}

int main() {
    // Устанавливаем уровень логов
    bm_log_set_level(BM_LOG_LEVEL_DEBUG);
    bm_log_info("=== Burymetal Unified Example ===");

    // --- Создание устройства (CPU для примера) ---
    BMDevice* dev = bm_create_device(BM_CPU);
    if (!dev) {
        bm_log_error("Ошибка создания устройства: %s", bm_get_last_error());
        return 1;
    }

    // --- Создание буфера ---
    float input[N] = {1, 2, 3, 4};
    float output[N] = {0};
    BMBuffer* buf = bm_alloc_buffer(dev, sizeof(input));
    if (!buf) {
        bm_log_error("Ошибка выделения буфера: %s", bm_get_last_error());
        bm_destroy_device(dev);
        return 1;
    }

    // --- Загрузка данных в буфер ---
    if (bm_upload_data(buf, input) != BM_SUCCESS) {
        bm_log_error("Ошибка загрузки данных: %s", bm_get_last_error());
        bm_free_buffer(buf);
        bm_destroy_device(dev);
        return 1;
    }

    // --- Регистрация и запуск ядра ---
    BMKernel* kernel = bm_register_kernel(dev, "double_kernel", double_kernel);
    if (!kernel) {
        bm_log_error("Ошибка регистрации ядра: %s", bm_get_last_error());
        bm_free_buffer(buf);
        bm_destroy_device(dev);
        return 1;
    }

    if (bm_launch_kernel(kernel, buf, N) != BM_SUCCESS) {
        bm_log_error("Ошибка запуска ядра: %s", bm_get_last_error());
        bm_destroy_kernel(kernel);
        bm_free_buffer(buf);
        bm_destroy_device(dev);
        return 1;
    }

    // --- Скачивание данных обратно ---
    if (bm_download_data(buf, output) != BM_SUCCESS) {
        bm_log_error("Ошибка скачивания данных: %s", bm_get_last_error());
        bm_destroy_kernel(kernel);
        bm_free_buffer(buf);
        bm_destroy_device(dev);
        return 1;
    }

    // --- Вывод результата ---
    bm_log_info("Результат выполнения ядра:");
    for (size_t i = 0; i < N; i++) {
        printf("%.1f ", output[i]);
    }
    printf("\n");

    // --- Очистка ресурсов ---
    bm_destroy_kernel(kernel);
    bm_free_buffer(buf);
    bm_destroy_device(dev);

    return 0;
}
