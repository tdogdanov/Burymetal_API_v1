#include <stdio.h>
#include <stdlib.h>
#include "burymetal/burymetal.h"

// Размер матриц
#define N 4

int main() {
    // Создание устройства (используем первый доступный backend)
    BMDevice* dev = bm_create_device();
    if (!dev) {
        fprintf(stderr, "Ошибка: не удалось создать устройство!\n");
        return 1;
    }

    BMBackend* backend = dev->backend;

    // Создаем буферы для матриц A, B и C
    BMBuffer* A = malloc(sizeof(BMBuffer));
    BMBuffer* B = malloc(sizeof(BMBuffer));
    BMBuffer* C = malloc(sizeof(BMBuffer));

    backend->alloc_buffer(backend, A, N * N * sizeof(float));
    backend->alloc_buffer(backend, B, N * N * sizeof(float));
    backend->alloc_buffer(backend, C, N * N * sizeof(float));

    // Инициализация матриц на хосте
    float host_A[N*N], host_B[N*N];
    for (int i = 0; i < N*N; ++i) {
        host_A[i] = i + 1;
        host_B[i] = (i + 1) * 0.5f;
    }

    // Копируем данные на устройство
    backend->copy_to_device(backend, A, host_A, N * N * sizeof(float));
    backend->copy_to_device(backend, B, host_B, N * N * sizeof(float));

    // Создаем ядро для матричного умножения
    BMKernel kernel;
    bm_kernel_init(&kernel, backend, "matmul");

    BMBuffer* inputs[] = { A, B };
    BMBuffer* outputs[] = { C };

    // Запускаем ядро
    backend->launch_kernel(backend, &kernel, inputs, 2, outputs, 1);

    // Копируем результат обратно на хост
    float host_C[N*N];
    backend->copy_to_host(backend, C, host_C, N * N * sizeof(float));

    // Выводим результат
    printf("Результат матричного умножения:\n");
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            printf("%6.2f ", host_C[i*N + j]);
        }
        printf("\n");
    }

    // Очистка
    backend->free_buffer(backend, A);
    backend->free_buffer(backend, B);
    backend->free_buffer(backend, C);
    free(A);
    free(B);
    free(C);

    bm_kernel_free(&kernel, backend);
    bm_destroy_device(dev);

    return 0;
}
