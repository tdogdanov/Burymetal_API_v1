// bm_device.c
#include "burymetal.h"
#include "bm_utils.h"
#include "bm_backend.h"

#include <stdlib.h>
#include <string.h>

// -----------------------------
// Создание устройства
// -----------------------------
BMResult bm_create_device(BMComputeTarget type, BMDevice** out_device) {
    if (!out_device) return BM_ERROR_INVALID_ARG;

    BMDevice* dev = (BMDevice*)malloc(sizeof(BMDevice));
    if (!dev) {
        bm_set_last_error("bm_create_device: не удалось выделить память");
        return BM_ERROR_NOMEM;
    }

    memset(dev, 0, sizeof(*dev));
    dev->type = type;

    // Инициализация backend
    if (type != BM_CPU) {
        BMResult res = bm_backend_init_device(dev);
        if (res != BM_OK) {
            free(dev);
            // bm_backend_init_device должен установить last_error
            return res;
        }
    }

    *out_device = dev;
    bm_log(BM_LOG_INFO, "Устройство создано: %s", 
           (type == BM_CPU) ? "CPU" : "GPU/Backend");
    return BM_OK;
}

// -----------------------------
// Уничтожение устройства
// -----------------------------
BMResult bm_destroy_device(BMDevice* device) {
    if (!device) return BM_OK;

    if (device->type != BM_CPU && device->backend_context) {
        BMResult res = bm_backend_destroy_device(device);
        if (res != BM_OK) {
            bm_set_last_error("bm_destroy_device: ошибка backend при уничтожении");
            return res;
        }
    }

    bm_log(BM_LOG_INFO, "Устройство уничтожено");
    free(device);
    return BM_OK;
}

// -----------------------------
// Получение информации о устройстве
// -----------------------------
BMResult bm_query_device(BMDevice* device, BMDeviceInfo* info) {
    if (!device || !info) return BM_ERROR_INVALID_ARG;

    if (device->type == BM_CPU) {
        info->name = "CPU";
        info->compute_units = 1;   // пример
        info->memory_size = 0;     // RAM не отслеживаем
        return BM_OK;
    }

    // Для GPU делегируем backend
    BMResult res = bm_backend_query_device(device, info);
    if (res != BM_OK) {
        bm_set_last_error("bm_query_device: ошибка backend");
        return res;
    }

    return BM_OK;
}
