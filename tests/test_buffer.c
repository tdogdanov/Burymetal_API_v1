// test_buffer.c
#include "burymetal.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_basic(BMDevice* dev) {
    size_t buf_size = 1024;
    BMBuffer* buffer = bm_alloc_buffer(dev, buf_size);
    assert(buffer && "bm_alloc_buffer failed");

    const char* msg = "Burymetal test buffer!";
    CHECK(bm_upload_data(buffer, msg) == BM_SUCCESS, "Ошибка записи данных");

    char out[64] = {0};
    CHECK(bm_download_data(buffer, out) == BM_SUCCESS, "Ошибка чтения данных");

    assert(strcmp(msg, out) == 0 && "buffer mismatch");

    bm_free_buffer(buffer);
}

static void test_offsets(BMDevice* dev) {
    size_t buf_size = 32;
    BMBuffer* buffer = bm_alloc_buffer(dev, buf_size);
    assert(buffer);

    // CPU backend может дать прямой доступ
    if (bm_get_host_ptr(buffer)) {
        memset(bm_get_host_ptr(buffer), 0xAA, buf_size);
    }

    const char* msg = "XYZ";
    CHECK(bm_write_buffer(buffer, msg, strlen(msg), 10) == BM_SUCCESS, "Ошибка записи с offset");

    char out[4] = {0};
    CHECK(bm_read_buffer(buffer, out, 3, 10) == BM_SUCCESS, "Ошибка чтения с offset");

    assert(strcmp(out, "XYZ") == 0);

    bm_free_buffer(buffer);
}

static void test_errors(BMDevice* dev) {
    BMBuffer* bad = bm_alloc_buffer(dev, 0);
    assert(bad == NULL);
    printf("expected error: %s\n", bm_get_last_error());

    bm_write_buffer(NULL, "x", 1, 0);
    printf("expected error: %s\n", bm_get_last_error());
}

int main() {
    printf("=== Burymetal Buffer Tests ===\n");

    BMDevice* dev = bm_create_device();
    assert(dev);

    test_basic(dev);
    test_offsets(dev);
    test_errors(dev);

    bm_destroy_device(dev);
    printf("All tests passed ✅\n");
    return 0;
}
