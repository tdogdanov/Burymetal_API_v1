#include "burymetal.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 4  // размер матриц N x N (для простого примера)

// Простое CPU-ядро: матричное умножение
void matmul_kernel(void* data, size_t count) {
    float* buf = (float*)data;
    float* A = buf;
    float* B = buf + N*N;
    float* C = buf + 2*N*N;

    for (size_t i = 0; i < N; i++) {
        for (size_t j = 0; j < N; j++) {
            float sum = 0.0f;
            for (size_t k = 0; k < N; k++) {
                sum += A[i*N + k] * B[k*N + j];
            }
            C[i*N + j] = sum;
        }
    }
}

int main() {
    printf("=== AI Kernel: Matrix Multiplication ===\n");

    // --- Создаём устройство ---
    BMDevice* device = bm_create_device();
    if (!device) {
        fprintf(stderr, "Ошибка: %s\n", bm_get_last_error());
        return 1;
    }

    // --- Регистрация ядра ---
    BMKernel* kernel = bm_register_kernel(device, "matmul_kernel", matmul_kernel);
    if (!kernel) {
        fprintf(stderr, "Ошибка: %s\n", bm_get_last_error());
        bm_destroy_device(device);
        return 1;
    }

    // --- Выделение буфера для A, B, C ---
    size_t buf_size = 3 * N * N * sizeof(float);
    BMBuffer* buf = bm_alloc_buffer(device, buf_size);
    if (!buf) {
        fprintf(stderr, "Ошибка: %s\n", bm_get_last_error());
        bm_unregister_kernel(kernel);
        bm_destroy_device(device);
        return 1;
    }

    // --- Инициализация матриц ---
    float* data = (float*)malloc(buf_size);
    srand((unsigned)time(NULL));
    for (int i = 0; i < N*N; i++) {
        data[i] = (float)(rand() % 10);       // A
        data[N*N + i] = (float)(rand() % 10); // B
        data[2*N*N + i] = 0.0f;               // C
    }

    bm_write_buffer(buf, data, buf_size, 0);
    free(data);

    // --- Запуск ядра ---
    bm_launch_kernel(kernel, buf, N*N);
    printf("Ядро выполнено ✅\n");

    // --- Скачиваем результат ---
    float result[3*N*N];
    bm_read_buffer(buf, result, buf_size, 0);

    // --- Вывод матрицы C ---
    printf("Результат C = A*B:\n");
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            printf("%6.1f ", result[2*N*N + i*N + j]);
        }
        printf("\n");
    }

    // --- Очистка ---
    bm_free_buffer(buf);
    bm_unregister_kernel(kernel);
    bm_destroy_device(device);

    return 0;
}
