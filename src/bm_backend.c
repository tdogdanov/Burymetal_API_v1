// test_gpu_double.c
#include "burymetal.h"
#include <stdio.h>
#include <stdlib.h>

#define N 16

// --- CPU ядро ---
void double_cpu(void* data, size_t count) {
    float* arr = (float*)data;
    for (size_t i = 0; i < count; i++) arr[i] *= 2.0f;
}

#ifdef BM_USE_CUDA
#include <cuda_runtime.h>

// --- CUDA ядро ---
__global__ void double_kernel_cuda(float* data, size_t count) {
    size_t idx = threadIdx.x + blockIdx.x * blockDim.x;
    if (idx < count) data[idx] *= 2.0f;
}

// Обёртка запуска CUDA ядра
void launch_cuda_kernel(BMBuffer* buf, size_t count) {
    int threads = 256;
    int blocks = (count + threads - 1) / threads;
    double_kernel_cuda<<<blocks, threads>>>((float*)buf->gpu_ptr, count);
    cudaDeviceSynchronize();
}
#endif

int main() {
    bm_log_set_level(BM_LOG_LEVEL_INFO);
    bm_log_info("=== Burymetal CPU+GPU Double Example ===");

    // --- Создаём устройство ---
    BMDevice* dev = bm_create_device(BM_AUTO);
    if (!dev) {
        fprintf(stderr, "Ошибка создания устройства: %s\n", bm_get_last_error());
        return 1;
    }

    // --- Создаём буфер ---
    BMBuffer* buf = bm_alloc_buffer(dev, N * sizeof(float));
    if (!buf) {
        fprintf(stderr, "Ошибка выделения буфера: %s\n", bm_get_last_error());
        bm_destroy_device(dev);
        return 1;
    }

    // --- Инициализируем данные ---
    float input[N];
    for (int i = 0; i < N; i++) input[i] = (float)i;
#ifdef BM_USE_CUDA
    if (dev->type == BM_CPU) {
        bm_upload_data(buf, input);
    } else {
        cudaMemcpy(buf->gpu_ptr, input, N*sizeof(float), cudaMemcpyHostToDevice);
    }
#else
    bm_upload_data(buf, input);
#endif

    // --- Запускаем ядро ---
    if (dev->type == BM_CPU) {
        double_cpu(buf->data, N);
    }
#ifdef BM_USE_CUDA
    else {
        launch_cuda_kernel(buf, N);
    }
#endif

    // --- Скачиваем результат ---
    float output[N] = {0};
#ifdef BM_USE_CUDA
    if (dev->type == BM_CPU) {
        bm_download_data(buf, output);
    } else {
        cudaMemcpy(output, buf->gpu_ptr, N*sizeof(float), cudaMemcpyDeviceToHost);
    }
#else
    bm_download_data(buf, output);
#endif

    // --- Вывод результата ---
    printf("Результат: ");
    for (int i = 0; i < N; i++) printf("%.1f ", output[i]);
    printf("\n");

    bm_free_buffer(buf);
    bm_destroy_device(dev);
    return 0;
}
