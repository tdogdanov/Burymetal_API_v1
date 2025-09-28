#ifndef BM_MEM_ALLOC_H
#define BM_MEM_ALLOC_H

#include <stddef.h>
#include "bm_types.h"
#include "bm_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------
// CPU память
// -----------------------------

/**
 * Выделение CPU буфера
 * @param size Размер буфера в байтах
 * @param out_ptr Указатель для возврата адреса выделенной памяти
 * @return BM_SUCCESS или BM_ERROR
 */
BMResult bm_cpu_alloc(size_t size, void** out_ptr);

/**
 * Освобождение CPU буфера
 * @param ptr Указатель на память
 * @return BM_SUCCESS или BM_ERROR
 */
BMResult bm_cpu_free(void* ptr);

// -----------------------------
// GPU память
// -----------------------------

/**
 * Выделение GPU буфера
 * @param device Устройство GPU
 * @param size Размер буфера в байтах
 * @param out_ptr Указатель для возврата адреса или handle
 * @return BM_SUCCESS или BM_ERROR
 */
BMResult bm_gpu_alloc(BMDevice* device, size_t size, void** out_ptr);

/**
 * Освобождение GPU буфера
 * @param device Устройство GPU
 * @param ptr Указатель на память или handle
 * @return BM_SUCCESS или BM_ERROR
 */
BMResult bm_gpu_free(BMDevice* device, void* ptr);

// -----------------------------
// Копирование данных
// -----------------------------

/**
 * Копирование данных с CPU (host) в буфер устройства
 */
BMResult bm_memcpy_host_to_device(BMBuffer* buffer, const void* src, size_t size);

/**
 * Копирование данных из буфера устройства в CPU (host)
 */
BMResult bm_memcpy_device_to_host(BMBuffer* buffer, void* dst, size_t size);

/**
 * Копирование данных между буферами устройства
 */
BMResult bm_memcpy_device_to_device(BMBuffer* dst, BMBuffer* src, size_t size);

#ifdef __cplusplus
}
#endif

#endif // BM_MEM_ALLOC_H
