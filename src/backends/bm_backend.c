// bm_backend.c
#include "burymetal.h"
#include "bm_utils.h"
#include <stdlib.h>
#include <string.h>

#ifdef BM_USE_CUDA
#include <cuda_runtime.h>
#endif

#ifdef BM_USE_OPENCL
#include <CL/cl.h>
#endif

#ifdef BM_USE_VULKAN
#include <vulkan/vulkan.h>
#endif

// -----------------------------------------
// CPU Backend
// -----------------------------------------
void* bm_backend_alloc_buffer_cpu(size_t size) {
    void* ptr = malloc(size);
    if (!ptr) bm_set_last_error("CPU backend: не удалось выделить память");
    return ptr;
}

void bm_backend_free_buffer_cpu(void* ptr) {
    if (ptr) free(ptr);
}

BMResult bm_backend_upload_data_cpu(void* dst, const void* src, size_t size) {
    if (!dst || !src) return BM_ERROR_INVALID_ARG;
    memcpy(dst, src, size);
    return BM_OK;
}

BMResult bm_backend_download_data_cpu(const void* src, void* dst, size_t size) {
    if (!dst || !src) return BM_ERROR_INVALID_ARG;
    memcpy(dst, src, size);
    return BM_OK;
}

BMResult bm_backend_launch_kernel_cpu(BMKernel* kernel, BMBuffer* buf, size_t count) {
    if (!kernel || !buf || !buf->data || !kernel->cpu_func) return BM_ERROR_INVALID_ARG;
    kernel->cpu_func(buf->data, count);
    return BM_OK;
}

// -----------------------------------------
// GPU Backends
// -----------------------------------------
#ifdef BM_USE_CUDA
void* bm_backend_alloc_buffer_cuda(BMDevice* dev, size_t size) {
    (void)dev;
    void* dev_ptr = NULL;
    cudaError_t err = cudaMalloc(&dev_ptr, size);
    if (err != cudaSuccess) {
        bm_set_last_error("CUDA alloc error: %s", cudaGetErrorString(err));
        return NULL;
    }
    return dev_ptr;
}

void bm_backend_free_buffer_cuda(void* dev_ptr) {
    if (dev_ptr) cudaFree(dev_ptr);
}

BMResult bm_backend_upload_data_cuda(void* dst, const void* src, size_t size) {
    cudaError_t err = cudaMemcpy(dst, src, size, cudaMemcpyHostToDevice);
    if (err != cudaSuccess) {
        bm_set_last_error("CUDA upload error: %s", cudaGetErrorString(err));
        return BM_ERROR;
    }
    return BM_OK;
}

BMResult bm_backend_download_data_cuda(const void* src, void* dst, size_t size) {
    cudaError_t err = cudaMemcpy(dst, src, size, cudaMemcpyDeviceToHost);
    if (err != cudaSuccess) {
        bm_set_last_error("CUDA download error: %s", cudaGetErrorString(err));
        return BM_ERROR;
    }
    return BM_OK;
}

BMResult bm_backend_launch_kernel_cuda(BMKernel* kernel, BMBuffer* buf, size_t count) {
    (void)kernel; (void)buf; (void)count;
    // Здесь должен быть вызов cudaLaunchKernel с kernel->backend_kernel
    return BM_OK;
}
#endif // BM_USE_CUDA

// TODO: OpenCL Backend
#ifdef BM_USE_OPENCL
void* bm_backend_alloc_buffer_cl(BMDevice* dev, size_t size) { return NULL; }
void bm_backend_free_buffer_cl(void* ptr) {}
BMResult bm_backend_upload_data_cl(void* dst, const void* src, size_t size) { return BM_OK; }
BMResult bm_backend_download_data_cl(const void* src, void* dst, size_t size) { return BM_OK; }
BMResult bm_backend_launch_kernel_cl(BMKernel* kernel, BMBuffer* buf, size_t count) { return BM_OK; }
#endif

// TODO: Vulkan Backend
#ifdef BM_USE_VULKAN
void* bm_backend_alloc_buffer_vk(BMDevice* dev, size_t size) { return NULL; }
void bm_backend_free_buffer_vk(void* ptr) {}
BMResult bm_backend_upload_data_vk(void* dst, const void* src, size_t size) { return BM_OK; }
BMResult bm_backend_download_data_vk(const void* src, void* dst, size_t size) { return BM_OK; }
BMResult bm_backend_launch_kernel_vk(BMKernel* kernel, BMBuffer* buf, size_t count) { return BM_OK; }
#endif

// -----------------------------------------
// Унифицированный интерфейс
// -----------------------------------------
void* bm_backend_alloc_buffer(BMDevice* dev, size_t size) {
    switch(dev->type) {
        case BM_CPU: return bm_backend_alloc_buffer_cpu(size);
#ifdef BM_USE_CUDA
        case BM_NVIDIA: return bm_backend_alloc_buffer_cuda(dev, size);
#endif
#ifdef BM_USE_OPENCL
        case BM_AMD: return bm_backend_alloc_buffer_cl(dev, size);
#endif
#ifdef BM_USE_VULKAN
        case BM_INTEL: return bm_backend_alloc_buffer_vk(dev, size);
#endif
        default:
            bm_set_last_error("Unsupported backend type");
            return NULL;
    }
}

void bm_backend_free_buffer(BMBuffer* buf) {
    if (!buf || !buf->gpu_ptr) return;
    switch(buf->device->type) {
        case BM_CPU: bm_backend_free_buffer_cpu(buf->gpu_ptr); break;
#ifdef BM_USE_CUDA
        case BM_NVIDIA: bm_backend_free_buffer_cuda(buf->gpu_ptr); break;
#endif
#ifdef BM_USE_OPENCL
        case BM_AMD: bm_backend_free_buffer_cl(buf->gpu_ptr); break;
#endif
#ifdef BM_USE_VULKAN
        case BM_INTEL: bm_backend_free_buffer_vk(buf->gpu_ptr); break;
#endif
        default: bm_set_last_error("Unsupported backend type"); break;
    }
}

BMResult bm_backend_upload_data(BMBuffer* buf, const void* data) {
    if (!buf || !buf->gpu_ptr) return BM_ERROR_INVALID_ARG;
    switch(buf->device->type) {
        case BM_CPU: return bm_backend_upload_data_cpu(buf->data, data, buf->size);
#ifdef BM_USE_CUDA
        case BM_NVIDIA: return bm_backend_upload_data_cuda(buf->gpu_ptr, data, buf->size);
#endif
#ifdef BM_USE_OPENCL
        case BM_AMD: return bm_backend_upload_data_cl(buf->gpu_ptr, data, buf->size);
#endif
#ifdef BM_USE_VULKAN
        case BM_INTEL: return bm_backend_upload_data_vk(buf->gpu_ptr, data, buf->size);
#endif
        default: return BM_ERROR_UNSUPPORTED;
    }
}

BMResult bm_backend_download_data(BMBuffer* buf, void* data) {
    if (!buf || !buf->gpu_ptr) return BM_ERROR_INVALID_ARG;
    switch(buf->device->type) {
        case BM_CPU: return bm_backend_download_data_cpu(buf->data, data, buf->size);
#ifdef BM_USE_CUDA
        case BM_NVIDIA: return bm_backend_download_data_cuda(buf->gpu_ptr, data, buf->size);
#endif
#ifdef BM_USE_OPENCL
        case BM_AMD: return bm_backend_download_data_cl(buf->gpu_ptr, data, buf->size);
#endif
#ifdef BM_USE_VULKAN
        case BM_INTEL: return bm_backend_download_data_vk(buf->gpu_ptr, data, buf->size);
#endif
        default: return BM_ERROR_UNSUPPORTED;
    }
}

BMResult bm_backend_launch_kernel(BMKernel* kernel, BMBuffer* buf, size_t count) {
    if (!kernel || !buf) return BM_ERROR_INVALID_ARG;

    switch(kernel->device->type) {
        case BM_CPU: return bm_backend_launch_kernel_cpu(kernel, buf, count);
#ifdef BM_USE_CUDA
        case BM_NVIDIA: return bm_backend_launch_kernel_cuda(kernel, buf, count);
#endif
#ifdef BM_USE_OPENCL
        case BM_AMD: return bm_backend_launch_kernel_cl(kernel, buf, count);
#endif
#ifdef BM_USE_VULKAN
        case BM_INTEL: return bm_backend_launch_kernel_vk(kernel, buf, count);
#endif
        default: return BM_ERROR_UNSUPPORTED;
    }
}
