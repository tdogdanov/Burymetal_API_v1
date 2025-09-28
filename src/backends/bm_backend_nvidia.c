#include "burymetal.h"
#include "bm_types.h"
#include "bm_backend.h"
#include "bm_utils.h"
#include <cuda_runtime.h>
#include <stdlib.h>
#include <stdio.h>

// ----------------------------------------
// CUDA Kernel для удвоения чисел
// ----------------------------------------
__global__ void double_kernel_cuda(float* data, size_t count) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < count) data[idx] *= 2.0f;
}

// ----------------------------------------
// Создание устройства
// ----------------------------------------
BMDevice* bm_backend_create_device(BMComputeTarget type) {
    int deviceCount = 0;
    cudaError_t err = cudaGetDeviceCount(&deviceCount);
    if (err != cudaSuccess || deviceCount == 0) {
        bm_set_last_error("[CUDA] Нет доступных GPU");
        return NULL;
    }

    BMDevice* dev = (BMDevice*)malloc(sizeof(BMDevice));
    if (!dev) {
        bm_set_last_error("[CUDA] Ошибка выделения памяти для BMDevice");
        return NULL;
    }

    dev->type = BM_NVIDIA;
    dev->id = 0;
    dev->handle = NULL;
    snprintf(dev->name, sizeof(dev->name), "NVIDIA GPU (id=%d)", dev->id);

    cudaSetDevice(dev->id);
    bm_log_info("[CUDA] Устройство создано: %s", dev->name);
    return dev;
}

// ----------------------------------------
// Освобождение устройства
// ----------------------------------------
void bm_backend_destroy_device(BMDevice* device) {
    if (!device) return;
    bm_log_info("[CUDA] Устройство уничтожено: %s", device->name);
    free(device);
}

// ----------------------------------------
// Буферы GPU
// ----------------------------------------
BMBuffer* bm_backend_alloc_buffer(BMDevice* device, size_t size) {
    if (!device || size == 0) {
        bm_set_last_error("[CUDA] alloc_buffer: некорректные аргументы");
        return NULL;
    }

    BMBuffer* buf = (BMBuffer*)malloc(sizeof(BMBuffer));
    if (!buf) {
        bm_set_last_error("[CUDA] Ошибка выделения памяти для BMBuffer");
        return NULL;
    }

    buf->device = device;
    buf->size = size;
    cudaError_t err = cudaMalloc(&buf->gpu_ptr, size);
    if (err != cudaSuccess) {
        bm_set_last_error("[CUDA] Ошибка cudaMalloc: %s", cudaGetErrorString(err));
        free(buf);
        return NULL;
    }

    bm_log_info("[CUDA] Выделен буфер %zu байт", size);
    return buf;
}

void bm_backend_free_buffer(BMBuffer* buffer) {
    if (!buffer) return;
    if (buffer->gpu_ptr) cudaFree(buffer->gpu_ptr);
    bm_log_info("[CUDA] Освобождён буфер %zu байт", buffer->size);
    free(buffer);
}

// ----------------------------------------
// Загрузка/скачивание данных
// ----------------------------------------
BMStatus bm_backend_upload_data(BMBuffer* buffer, const void* data) {
    if (!buffer || !buffer->gpu_ptr || !data) {
        bm_set_last_error("[CUDA] upload_data: некорректные аргументы");
        return BM_STATUS_ERROR;
    }

    cudaError_t err = cudaMemcpy(buffer->gpu_ptr, data, buffer->size, cudaMemcpyHostToDevice);
    if (err != cudaSuccess) {
        bm_set_last_error("[CUDA] upload_data: %s", cudaGetErrorString(err));
        return BM_STATUS_ERROR;
    }

    bm_log_debug("[CUDA] Загружены данные в буфер (%zu байт)", buffer->size);
    return BM_STATUS_OK;
}

BMStatus bm_backend_download_data(BMBuffer* buffer, void* data) {
    if (!buffer || !buffer->gpu_ptr || !data) {
        bm_set_last_error("[CUDA] download_data: некорректные аргументы");
        return BM_STATUS_ERROR;
    }

    cudaError_t err = cudaMemcpy(data, buffer->gpu_ptr, buffer->size, cudaMemcpyDeviceToHost);
    if (err != cudaSuccess) {
        bm_set_last_error("[CUDA] download_data: %s", cudaGetErrorString(err));
        return BM_STATUS_ERROR;
    }

    bm_log_debug("[CUDA] Скачаны данные из буфера (%zu байт)", buffer->size);
    return BM_STATUS_OK;
}

// ----------------------------------------
// Ядра
// ----------------------------------------
BMKernel* bm_backend_load_kernel(BMDevice* device, const char* kernel_path) {
    (void)device;
    (void)kernel_path;

    BMKernel* kernel = (BMKernel*)malloc(sizeof(BMKernel));
    if (!kernel) {
        bm_set_last_error("[CUDA] Ошибка выделения памяти для BMKernel");
        return NULL;
    }

    kernel->device = device;
    kernel->kernel_ptr = (void*)double_kernel_cuda;
    kernel->cpu_func = NULL; // CPU fallback можно назначить
    snprintf(kernel->name, sizeof(kernel->name), "double_kernel_cuda");

    bm_log_info("[CUDA] Kernel загружен: %s", kernel->name);
    return kernel;
}

BMStatus bm_backend_run_kernel(BMKernel* kernel, BMBuffer* buffer, size_t count) {
    if (!kernel || !buffer || !buffer->gpu_ptr) {
        bm_set_last_error("[CUDA] run_kernel: некорректные аргументы");
        return BM_STATUS_ERROR;
    }

    int threads = 256;
    int blocks = (int)((count + threads - 1) / threads);

    double_kernel_cuda<<<blocks, threads>>>((float*)buffer->gpu_ptr, count);
    cudaError_t err = cudaDeviceSynchronize();
    if (err != cudaSuccess) {
        bm_set_last_error("[CUDA] run_kernel: %s", cudaGetErrorString(err));
        return BM_STATUS_ERROR;
    }

    bm_log_info("[CUDA] Kernel выполнен на %zu элементов", count);
    return BM_STATUS_OK;
}

void bm_backend_destroy_kernel(BMKernel* kernel) {
    if (!kernel) return;
    bm_log_info("[CUDA] Kernel уничтожен: %s", kernel->name);
    free(kernel);
}

// ----------------------------------------
// Синхронизация
// ----------------------------------------
BMStatus bm_backend_sync(BMDevice* device) {
    if (!device) return BM_STATUS_ERROR;

    cudaError_t err = cudaDeviceSynchronize();
    if (err != cudaSuccess) {
        bm_set_last_error("[CUDA] sync: %s", cudaGetErrorString(err));
        return BM_STATUS_ERROR;
    }

    bm_log_debug("[CUDA] sync выполнен");
    return BM_STATUS_OK;
}

// ----------------------------------------
// Query устройства
// ----------------------------------------
BMStatus bm_backend_query_device(BMDevice* device, BMDeviceInfo* info) {
    if (!device || !info) return BM_STATUS_ERROR;

    cudaDeviceProp prop;
    cudaError_t err = cudaGetDeviceProperties(&prop, device->id);
    if (err != cudaSuccess) {
        bm_set_last_error("[CUDA] query_device: %s", cudaGetErrorString(err));
        return BM_STATUS_ERROR;
    }

    info->type = BM_NVIDIA;
    snprintf(info->name, sizeof(info->name), "%s", prop.name);
    info->memory_total = (uint32_t)(prop.totalGlobalMem / (1024 * 1024));
    info->memory_free = 0; // точное значение можно получить через cudaMemGetInfo
    info->compute_units = prop.multiProcessorCount;

    bm_log_info("[CUDA] Запрос информации об устройстве: %s", info->name);
    return BM_STATUS_OK;
}
