// bm_raii.h
#ifndef BM_RAII_H
#define BM_RAII_H

#include "burymetal.h"

// ============================
// RAII обёртки для устройств, буферов и ядер
// ============================

typedef struct { BMDevice* dev; } BMDeviceGuard;
typedef struct { BMBuffer* buf; } BMBufferGuard;
typedef struct { BMKernel* k; } BMKernelGuard;

// ----------------------------
// Cleanup-функции
// ----------------------------
static inline void bm_cleanup_device(BMDeviceGuard* g) {
    if (g && g->dev) {
        bm_destroy_device(g->dev);
        g->dev = NULL;
    }
}

static inline void bm_cleanup_buffer(BMBufferGuard* g) {
    if (g && g->buf) {
        bm_free_buffer(g->buf);
        g->buf = NULL;
    }
}

static inline void bm_cleanup_kernel(BMKernelGuard* g) {
    if (g && g->k) {
        bm_destroy_kernel(g->k);
        g->k = NULL;
    }
}

// ----------------------------
// Макросы для автоматического вызова cleanup
// ----------------------------
#ifdef __GNUC__
#define BM_AUTO_DEVICE  __attribute__((cleanup(bm_cleanup_device))) BMDeviceGuard
#define BM_AUTO_BUFFER  __attribute__((cleanup(bm_cleanup_buffer))) BMBufferGuard
#define BM_AUTO_KERNEL  __attribute__((cleanup(bm_cleanup_kernel))) BMKernelGuard
#else
#warning "RAII автоматический cleanup работает только в GCC/Clang"
#define BM_AUTO_DEVICE  BMDeviceGuard
#define BM_AUTO_BUFFER  BMBufferGuard
#define BM_AUTO_KERNEL  BMKernelGuard
#endif

#endif // BM_RAII_H
