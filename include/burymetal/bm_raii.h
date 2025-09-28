#ifndef BM_RALL_H
#define BM_RALL_H

#include "burymetal.h"

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------
// RAII-обёртки для C (упрощённая структура)
// -----------------------------

typedef struct BMDeviceRAII {
    BMDevice* device;
} BMDeviceRAII;

typedef struct BMBufferRAII {
    BMBuffer* buffer;
} BMBufferRAII;

typedef struct BMKernelRAII {
    BMKernel* kernel;
} BMKernelRAII;

// -----------------------------
// Функции инициализации/освобождения
// -----------------------------

BMDeviceRAII bm_device_create_raii(BMComputeTarget type);
void bm_device_destroy_raii(BMDeviceRAII* dev);

BMBufferRAII bm_buffer_alloc_raii(BMDevice* device, size_t size);
void bm_buffer_free_raii(BMBufferRAII* buf);

BMKernelRAII bm_kernel_load_raii(BMDevice* device, const char* path);
void bm_kernel_destroy_raii(BMKernelRAII* kern);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // BM_RALL_H
