#ifndef BURYMETAL_H
#define BURYMETAL_H

#include <stddef.h>
#include "bm_types.h"

#ifdef __cplusplus
extern "C" {
#endif

// --- Стандартные коды ошибок ---
typedef enum {
    BM_OK = 0,
    BM_ERROR_NOMEM,
    BM_ERROR_INVALID_ARG,
    BM_ERROR_UNSUPPORTED,
    BM_ERROR_DEVICE_LOST,
    BM_ERROR_INTERNAL,
} BMResult;

const char* bm_result_string(BMResult code);

// --- Устройство ---
BMResult bm_create_device(BMComputeTarget type, BMDevice** out_device);
BMResult bm_destroy_device(BMDevice* device); // безопасно для NULL
BMResult bm_query_device(BMDevice* device, BMDeviceInfo* info);

// --- Буферы ---
BMResult bm_alloc_buffer(BMDevice* device, size_t size, BMBuffer** out_buffer);
BMResult bm_free_buffer(BMBuffer* buffer); // безопасно для NULL
BMResult bm_upload_data(BMBuffer* buffer, const void* data, size_t length);
BMResult bm_download_data(BMBuffer* buffer, void* data, size_t length);

// --- Ядра (Kernel) ---
BMResult bm_load_kernel(BMDevice* device, const char* kernel_path, BMKernel** out_kernel);
BMResult bm_launch_kernel(BMKernel* kernel, BMBuffer* buffer, size_t count);
BMResult bm_destroy_kernel(BMKernel* kernel); // безопасно для NULL

#ifdef __cplusplus
} // extern "C"
#endif

#endif // BURYMETAL_H
