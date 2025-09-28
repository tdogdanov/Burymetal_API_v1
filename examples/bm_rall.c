#include "bm_rall.h"
#include "bm_utils.h"

// -----------------------------
// BMDeviceRAII
// -----------------------------
BMDeviceRAII bm_device_create_raii(BMComputeTarget type) {
    BMDeviceRAII raii = {0};
    BMDevice* device = NULL;
    BMResult res = bm_create_device(type, &device);
    if (res != BM_OK) {
        bm_log(BM_LOG_ERROR, "Failed to create device: %s", bm_result_string(res));
        return raii; // device == NULL
    }
    raii.device = device;
    return raii;
}

void bm_device_destroy_raii(BMDeviceRAII* dev) {
    if (!dev || !dev->device) return;
    bm_destroy_device(dev->device);
    dev->device = NULL;
}

// -----------------------------
// BMBufferRAII
// -----------------------------
BMBufferRAII bm_buffer_alloc_raii(BMDevice* device, size_t size) {
    BMBufferRAII raii = {0};
    if (!device || size == 0) {
        bm_set_last_error("bm_buffer_alloc_raii: invalid arguments");
        return raii;
    }
    BMBuffer* buffer = NULL;
    BMResult res = bm_alloc_buffer(device, size, &buffer);
    if (res != BM_OK) {
        bm_log(BM_LOG_ERROR, "Failed to allocate buffer: %s", bm_result_string(res));
        return raii;
    }
    raii.buffer = buffer;
    return raii;
}

void bm_buffer_free_raii(BMBufferRAII* buf) {
    if (!buf || !buf->buffer) return;
    bm_free_buffer(buf->buffer);
    buf->buffer = NULL;
}

// -----------------------------
// BMKernelRAII
// -----------------------------
BMKernelRAII bm_kernel_load_raii(BMDevice* device, const char* path) {
    BMKernelRAII raii = {0};
    if (!device || !path) {
        bm_set_last_error("bm_kernel_load_raii: invalid arguments");
        return raii;
    }
    BMKernel* kernel = NULL;
    BMResult res = bm_load_kernel(device, path, &kernel);
    if (res != BM_OK) {
        bm_log(BM_LOG_ERROR, "Failed to load kernel: %s", bm_result_string(res));
        return raii;
    }
    raii.kernel = kernel;
    return raii;
}

void bm_kernel_destroy_raii(BMKernelRAII* kern) {
    if (!kern || !kern->kernel) return;
    bm_destroy_kernel(kern->kernel);
    kern->kernel = NULL;
}
