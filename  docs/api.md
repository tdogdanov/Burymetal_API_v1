Burymetal API Documentation
1. Overview

Burymetal is a cross-platform compute library for CPU and GPU (NVIDIA, AMD, Intel), designed for AI and general-purpose computations. It provides a unified API for devices, buffers, and compute kernels.

Key Features:

Unified API for CPU and GPU backends

Memory management via BMBuffer

Kernel execution via BMKernel

Safe resource management with RAII

Cross-platform (CPU, NVIDIA CUDA, AMD ROCm, Intel GPU)

2. Architecture
+-----------------+
|   User Program  |
+-----------------+
          |
          v
+-----------------+
|  Burymetal API  |
+-----------------+
   |          |
   v          v
CPU Backend   GPU Backend (NVIDIA / AMD / Intel)


Core (src/core/) — device, buffer, kernel, utils

Backends (src/backends/) — CPU/GPU implementations

RAII (bm_raii.h) — safe automatic resource management

3. Core Types
BMDevice

Represents a computational device.

BMDevice* bm_create_device();
void bm_destroy_device(BMDevice* dev);

BMBuffer

Represents memory on device or host.

typedef struct {
    void* ptr;
    size_t size;
} BMBuffer;


Operations:

backend->alloc_buffer(backend, buf, size);
backend->free_buffer(backend, buf);
backend->copy_to_device(backend, buf, host_data, size);
backend->copy_to_host(backend, buf, host_data, size);

BMKernel

Represents a compute kernel.

BMKernel kernel;
bm_kernel_init(&kernel, backend, "kernel_name");
backend->launch_kernel(backend, &kernel, inputs, num_inputs, outputs, num_outputs);
bm_kernel_free(&kernel, backend);

4. Backend Interface (bm_backend.h)
typedef struct BMBackend {
    BMBackendType type;
    BMStatus (*init)(struct BMBackend* backend);
    BMStatus (*shutdown)(struct BMBackend* backend);
    BMStatus (*alloc_buffer)(struct BMBackend*, BMBuffer*, size_t size);
    BMStatus (*free_buffer)(struct BMBackend*, BMBuffer*);
    BMStatus (*copy_to_device)(struct BMBackend*, BMBuffer*, const void* host_ptr, size_t size);
    BMStatus (*copy_to_host)(struct BMBackend*, BMBuffer*, void* host_ptr, size_t size);
    BMStatus (*launch_kernel)(struct BMBackend*, BMKernel*, BMBuffer** inputs, int num_inputs, BMBuffer** outputs, int num_outputs);
    void* user_data; // backend-specific
} BMBackend;


Supported Backends:

BM_BACKEND_CPU

BM_BACKEND_NVIDIA

BM_BACKEND_AMD

BM_BACKEND_INTEL

5. Examples
Simple Compute
#include "burymetal/burymetal.h"

int main() {
    BMDevice* dev = bm_create_device();
    // Create buffer, launch kernel...
    bm_destroy_device(dev);
}

Buffer Test

Demonstrates allocation, copy, and release of buffers on CPU/GPU.

MatMul AI

Demonstrates matrix multiplication via Burymetal, showing usage of multiple buffers and kernel launches.

6. Best Practices

Use bm_raii.h for automatic resource management

Always call bm_destroy_device() to release resources

Use unified buffer API to write backend-agnostic code

Keep kernels backend-independent whenever possible

7. Build and Installation

Using CMake:

mkdir build && cd build
cmake ..
make


scripts/build.sh can also be used for automated builds.

Examples are built with BUILD_EXAMPLES flag.

Tests can be run via make test if CTest is enabled.

8. Contributing

Follow folder structure for new backends or kernels

Add examples for new kernels in examples/

Unit tests go to tests/