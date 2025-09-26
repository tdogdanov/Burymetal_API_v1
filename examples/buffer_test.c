//buffer_test.c

#include "burymetal.h"
#include <stdio.h>
#include <string.h>

#define BUF_SIZE 256

int main() {
    printf("=== Пример buffer_test ===\n");

    // Создаем устройство
    BMDevice* dev = bm_create_device();
    if (!dev) {
        fprintf(stderr, "Ошибка: не удалось создать устройство!\n");
        return 1;
    }

    // Создаем буфер
    BMBuffer* buf = bm_alloc_buffer(dev, BUF_SIZE);
    if (!buf) {
        fprintf(stderr, "Ошибка: не удалось выделить буфер!\n");
        bm_destroy_device(dev);
        return 1;
    }

    const char* msg = "Hello Burymetal!";
    size_t msg_len = strlen(msg) + 1; // +1 для '\0'

    if (msg_len > BUF_SIZE) {
        fprintf(stderr, "Ошибка: сообщение превышает размер буфера!\n");
        bm_free_buffer(buf);
        bm_destroy_device(dev);
        return 1;
    }

    // Записываем в буфер
    if (bm_write_buffer(buf, msg, msg_len, 0) != BM_STATUS_OK) {
        fprintf(stderr, "Ошибка записи в буфер!\n");
        bm_free_buffer(buf);
        bm_destroy_device(dev);
        return 1;
    }

    // Читаем из буфера
    char out[BUF_SIZE] = {0};
    if (bm_read_buffer(buf, out, msg_len, 0) != BM_STATUS_OK) {
        fprintf(stderr, "Ошибка чтения из буфера!\n");
        bm_free_buffer(buf);
        bm_destroy_device(dev);
        return 1;
    }

    printf("Записано: \"%s\"\n", msg);
    printf("Считано:  \"%s\"\n", out);

    // Освобождаем ресурсы
    bm_free_buffer(buf);
    bm_destroy_device(dev);

    printf("=== Пример завершен успешно ===\n");
    return 0;
}
