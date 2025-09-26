// bm_device.c
#include "burymetal.h"
#include "bm_backend.h"
#include "bm_utils.h"
#include <stdlib.h>
#include <string.h>

// ----------------------------------------
// Создание устройства (CPU / GPU)
// ----------------------------------------
BMDevice* bm_create_device(BMComputeTarget type) {
    BMDevice* dev = bm_backend_create_device(type);
    if (!dev) {
        bm_set_last_error("[BM] bm_create_device: не удалось создать устройство");
        return NULL;
    }

    BMDeviceInfo info;
    bm_query_device(dev, &info);

    bm_log_info("[BM] Устройство создано: %s (type=%d)", info.name, dev->type);
    return dev;
}

// ----------------------------------------
// Уничтожение устройства
// ----------------------------------------
void bm_destroy_device(BMDevice* dev) {
    if (!dev) {
        bm_log_warn("[BM] bm_destroy_device: dev == NULL");
        return;
    }

    bm_backend_destroy_device(dev);
    bm_log_info("[BM] Устройство уничтожено: %s", dev->name);
}

// ----------------------------------------
// Получение информации об устройстве
// ----------------------------------------
void bm_query_device(BMDevice* dev, BMDeviceInfo* info) {
    if (!dev || !info) {
        bm_set_last_error("[BM] bm_query_device: некорректные аргументы");
        return;
    }

    memset(info, 0, sizeof(BMDeviceInfo));
    bm_backend_query_device(dev, info);

    bm_log_info("[BM] Информация об устройстве %s: CUs=%d, memory_total=%zu, memory_free=%zu",
                info->name, info->compute_units, info->memory_total, info->memory_free);
}
