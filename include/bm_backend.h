#ifndef BM_BACKEND_H
#define BM_BACKEND_H

#include "bm_types.h"

#ifdef __cplusplus
extern "C" {
#endif

// --- Backend-интерфейс ---
// Эти функции реализуют CPU / NVIDIA / AMD / Intel backends.
// Они не должны быть видны приложению (только внутренним *.c файлам).

BMDevice* bm_backend_create_device(BMComputeTarget type);
void bm_backend_destroy_device(BMDevice* device);
void bm_backend_query_device(BMDevice* device, BMDeviceInfo* info);

BMBuffer* bm_backend_alloc_buffer(BMDevice* device, size_t size);
void bm_backend_free_buffer(BMBuffer* buffer);
BMStatus bm_backend_upload(BMBuffer* buffer, const void* data);
BMStatus bm_backend_download(BMBuffer* buffer, void* data);

BMKernel* bm_backend_load_kernel(BMDevice* device, const char* kernel_path);
BMStatus bm_backend_launch_kernel(BMKernel* kernel, BMBuffer* buffer, size_t count);
void bm_backend_destroy_kernel(BMKernel* kernel);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // BM_BACKEND_H
