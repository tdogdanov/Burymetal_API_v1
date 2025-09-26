// test_Device.c
#include "burymetal.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    printf("=== Тест устройства Burymetal API ===\n");

    // --- Создание первого устройства ---
    BMDevice* device1 = bm_create_device(BM_AUTO);
    assert(device1 && "Ошибка: bm_create_device вернул NULL");

    BMDeviceInfo info1;
    if (bm_query_device(device1, &info1) == BM_SUCCESS) {
        printf("Первое устройство: %s, CU=%d, RAM=%zu\n",
               info1.name, info1.compute_units, info1.memory_size);
    } else {
        printf("Не удалось получить информацию о первом устройстве\n");
    }

    // --- Создание второго устройства ---
    BMDevice* device2 = bm_create_device(BM_AUTO);
    assert(device2 && "Ошибка: повторный вызов bm_create_device вернул NULL");

    BMDeviceInfo info2;
    if (bm_query_device(device2, &info2) == BM_SUCCESS) {
        printf("Второе устройство: %s, CU=%d, RAM=%zu\n",
               info2.name, info2.compute_units, info2.memory_size);
    } else {
        printf("Не удалось получить информацию о втором устройстве\n");
    }

    // --- Проверка буфера ---
    size_t buf_size = 256;
    BMBuffer* buf = bm_alloc_buffer(device1, buf_size);
    assert(buf && "Ошибка: bm_alloc_buffer вернул NULL");

    const char* msg = "Hello Burymetal!";
    bm_write_buffer(buf, msg, strlen(msg)+1, 0);

    char out[256] = {0};
    bm_read_buffer(buf, out, strlen(msg)+1, 0);
    assert(strcmp(msg, out) == 0 && "Ошибка: данные не совпадают");

    printf("Буфер успешно записан и прочитан ✅\n");

    // --- Очистка ресурсов ---
    bm_free_buffer(buf);
    bm_destroy_device(device1);
    bm_destroy_device(device2);
    printf("Тест устройства завершён успешно ✅\n");
    return 0;
}
