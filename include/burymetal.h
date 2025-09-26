#ifndef BURYMETAL_H
#define BURYMETAL_H

#include <stddef.h>
#include "bm_types.h"

#ifdef __cplusplus
extern "C" {
#endif

// --- API функций ---
BMDevice* bm_create_device(BMComputeTarget type);
void bm_destroy_device(BMDevice* device);
BMStatus bm_query_device(BMDevice* device, BMDeviceInfo* info);

BMBuffer* bm_alloc_buffer(BMDevice* device, size_t size);
void bm_free_buffer(BMBuffer* buffer);
BMStatus bm_upload_data(BMBuffer* buffer, const void* data);
BMStatus bm_download_data(BMBuffer* buffer, void* data);

BMKernel* bm_load_kernel(BMDevice* device, const char* kernel_path);
BMStatus bm_launch_kernel(BMKernel* kernel, BMBuffer* buffer, size_t count);
void bm_destroy_kernel(BMKernel* kernel);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // BURYMETAL_H
