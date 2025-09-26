// test_burymetal.c
#include "burymetal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(cond, msg) \
    if (!(cond)) { fprintf(stderr, "%s: %s\n", msg, bm_get_last_error()); goto cleanup; }

int main(int argc, char** argv) {
    printf("=== Burymetal Kernel Example ===\n");

    BMDevice* dev = NULL;
    BMBuffer* buf = NULL;
    BMKernel* kernel = NULL;

    // 1. Создание устройства
    dev = bm_create_device(BM_AUTO);
    CHECK(dev, "Ошибка создания устройства");

    BMDeviceInfo info;
    if (bm_query_device(dev, &info) == BM_SUCCESS) {
        printf("Устройство: %s, CU=%d, RAM=%zu\n", info.name, info.compute_units, info.memory_size);
    }

    // 2. Выделение буфера
    size_t buf_size = 16;
    buf = bm_alloc_buffer(dev, buf_size);
    CHECK(buf, "Ошибка выделения буфера");

    // 3. Подготовка данных
    unsigned char data[16];
    for (int i = 0; i < 16; i++) data[i] = (unsigned char)i;
    CHECK(bm_upload_data(buf, data) == BM_SUCCESS, "Ошибка загрузки данных");

    printf("До ядра: ");
    for (int i = 0; i < 16; i++) printf("%u ", data[i]);
    printf("\n");

    // 4. Загрузка ядра
    const char* kernel_name = (argc > 1) ? argv[1] : "double_kernel";
    kernel = bm_load_kernel(dev, kernel_name);
    CHECK(kernel, "Ошибка загрузки ядра");

    // 5. Запуск ядра
    CHECK(bm_launch_kernel(kernel, buf, buf_size) == BM_SUCCESS, "Ошибка запуска ядра");
    printf("Ядро '%s' выполнено.\n", kernel_name);

    // 6. Скачивание результата
    unsigned char out[16] = {0};
    CHECK(bm_download_data(buf, out) == BM_SUCCESS, "Ошибка скачивания данных");

    printf("После ядра: ");
    for (int i = 0; i < 16; i++) printf("%u ", out[i]);
    printf("\n");

    // 7. Проверка результата
    int ok = 1;
    for (int i = 0; i < 16; i++) {
        if (out[i] != data[i] * 2) { ok = 0; break; }
    }
    printf(ok ? "Тест успешно ✅\n" : "Ошибка ❌: ядро не преобразовало данные\n");

cleanup:
    if (kernel) bm_destroy_kernel(kernel);
    if (buf) bm_free_buffer(buf);
    if (dev) bm_destroy_device(dev);
    return 0;
}
