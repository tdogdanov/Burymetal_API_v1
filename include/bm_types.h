#ifndef BM_TYPES_H
#define BM_TYPES_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// --- Статус выполнения ---
typedef enum {
    BM_SUCCESS = 0,
    BM_ERROR   = -1
} BMStatus;

// --- Цели вычислений ---
typedef enum {
    BM_CPU,
    BM_NVIDIA,
    BM_AMD,
    BM_INTEL,
    BM_AUTO
} BMComputeTarget;

// --- Структуры API ---
typedef struct BMDevice {
    BMComputeTarget type;
    void* backend_context;   // приватные данные бэкенда
} BMDevice;

typedef struct BMBuffer {
    BMDevice* device;
    size_t size;
    void* data;
} BMBuffer;

typedef struct BMKernel {
    BMDevice* device;
    const char* path;
    void* backend_kernel;
} BMKernel;

typedef struct BMDeviceInfo {
    const char* name;
    int compute_units;
    size_t memory_size;
} BMDeviceInfo;

#ifdef __cplusplus
} // extern "C"
#endif

#endif // BM_TYPES_H
