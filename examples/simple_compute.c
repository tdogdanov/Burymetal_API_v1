#include "burymetal.h"
#include <stdio.h>

#define N 4

void double_kernel_cpu(void* data, size_t count) {
    float* arr = (float*)data;
    for (size_t i = 0; i < count; i++) arr[i] *= 2.0f;
}

int main() {
    BMDevice* dev = bm_create_device();
    if (!dev) { fprintf(stderr, "Ошибка создания устройства: %s\n", bm_get_last_error()); return 1; }

    float input[N] = {1,2,3,4};
    float output[N] = {0};

    BMBuffer* buf = bm_alloc_buffer(dev, sizeof(input));
    if (!buf) { fprintf(stderr, "Ошибка выделения буфера: %s\n", bm_get_last_error()); bm_destroy_device(dev); return 1; }

    if (bm_upload_data(buf, input) != BM_STATUS_OK) {
        fprintf(stderr, "Ошибка загрузки данных\n"); bm_free_buffer(buf); bm_destroy_device(dev); return 1;
    }

    BMKernel* kernel = bm_load_kernel(dev, "double_kernel");
    if (!kernel) { fprintf(stderr, "Ошибка загрузки ядра: %s\n", bm_get_last_error()); bm_free_buffer(buf); bm_destroy_device(dev); return 1; }

    if (dev->type == BM_CPU) {
        bm_download_data(buf, output);
        double_kernel_cpu(output, N);
    } else {
        bm_launch_kernel(kernel, buf, N);
        if (bm_download_data(buf, output) != BM_STATUS_OK) {
            fprintf(stderr, "Ошибка скачивания данных\n"); bm_destroy_kernel(kernel); bm_free_buffer(buf); bm_destroy_device(dev); return 1;
        }
    }

    printf("Результат: ");
    for (int i = 0; i < N; i++) printf("%.1f ", output[i]);
    printf("\n");

    bm_destroy_kernel(kernel);
    bm_free_buffer(buf);
    bm_destroy_device(dev);
    return 0;
}
