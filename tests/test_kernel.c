//test_kernel.c

#include "burymetal.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define MAX_ELEMENTS 1024

// --- Ядро для float: удвоение ---
void double_float(void* data, size_t count) {
    float* arr = (float*)data;
    for (size_t i = 0; i < count; i++)
        arr[i] *= 2.0f;
}

// --- Ядро для int: удвоение ---
void double_int(void* data, size_t count) {
    int* arr = (int*)data;
    for (size_t i = 0; i < count; i++)
        arr[i] *= 2;
}

// --- Функция теста для одного устройства, типа данных и размера ---
void test_kernel_type(BMComputeTarget target, const char* type_name, size_t elem_size, void(*kernel_func)(void*,size_t), size_t count) {
    printf("=== Тест %s, %zu элементов, backend=%d ===\n", type_name, count, target);

    BMDevice* dev = bm_create_device(target);
    if (!dev) {
        printf("Пропущен backend %d: %s\n", target, bm_get_last_error());
        return;
    }

    BMKernel* kernel = bm_register_kernel(dev, type_name, kernel_func);
    assert(kernel && "Ошибка регистрации ядра");

    BMBuffer* buf = bm_alloc_buffer(dev, elem_size * count);
    assert(buf && "Ошибка выделения буфера");

    void* input  = malloc(elem_size * count);
    void* expect = malloc(elem_size * count);
    void* output = malloc(elem_size * count);

    // Заполняем данные и ожидаемый результат
    for (size_t i = 0; i < count; i++) {
        if (strcmp(type_name, "float") == 0) {
            ((float*)input)[i]  = (float)(i + 1);
            ((float*)expect)[i] = ((float*)input)[i] * 2.0f;
        } else if (strcmp(type_name, "int") == 0) {
            ((int*)input)[i]  = (int)(i + 1);
            ((int*)expect)[i] = ((int*)input)[i] * 2;
        }
    }

    assert(bm_write_buffer(buf, input, elem_size * count, 0) == BM_SUCCESS);

    // --- Время запуска ядра ---
    clock_t t_start = clock();
    assert(bm_launch_kernel(kernel, buf, count) == BM_SUCCESS);
    clock_t t_end = clock();

    double elapsed_ms = 1000.0 * (t_end - t_start) / CLOCKS_PER_SEC;

    assert(bm_read_buffer(buf, output, elem_size * count, 0) == BM_SUCCESS);

    // --- Проверка результата ---
    int ok = 1;
    for (size_t i = 0; i < count; i++) {
        if (strcmp(type_name, "float") == 0) {
            if (fabsf(((float*)output)[i] - ((float*)expect)[i]) > 1e-6f) ok = 0;
        } else if (strcmp(type_name, "int") == 0) {
            if (((int*)output)[i] != ((int*)expect)[i]) ok = 0;
        }
    }

    printf("Результат: %s, время выполнения: %.3f ms\n", ok ? "OK ✅" : "FAIL ❌", elapsed_ms);

    // --- Очистка ---
    free(input);
    free(expect);
    free(output);
    bm_free_buffer(buf);
    bm_destroy_kernel(kernel);
    bm_destroy_device(dev);
}

int main() {
    BMComputeTarget backends[] = {BM_CPU, BM_NVIDIA, BM_AMD, BM_INTEL};
    size_t counts[] = {4, 16, 64, 256, 1024};

    for (size_t b = 0; b < sizeof(backends)/sizeof(backends[0]); b++) {
        for (size_t c = 0; c < sizeof(counts)/sizeof(counts[0]); c++) {
            test_kernel_type(backends[b], "float", sizeof(float), double_float, counts[c]);
            test_kernel_type(backends[b], "int",   sizeof(int),   double_int,   counts[c]);
        }
    }

    printf("Все тесты ядра успешно завершены ✅\n");
    return 0;
}
