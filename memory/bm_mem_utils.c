#include "bm_mem_utils.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/**
 * Обнуляет память
 * @param ptr Указатель на память
 * @param size Размер в байтах
 * @return BM_SUCCESS или BM_ERROR
 */
BMResult bm_mem_zero(void* ptr, size_t size) {
    if (!ptr || size == 0) {
        bm_set_last_error("bm_mem_zero: invalid arguments");
        return BM_ERROR;
    }
    memset(ptr, 0, size);
    return BM_SUCCESS;
}

/**
 * Копирует память
 * @param dst Указатель на целевую память
 * @param src Указатель на источник
 * @param size Размер в байтах
 * @return BM_SUCCESS или BM_ERROR
 */
BMResult bm_mem_copy(void* dst, const void* src, size_t size) {
    if (!dst || !src || size == 0) {
        bm_set_last_error("bm_mem_copy: invalid arguments");
        return BM_ERROR;
    }
    memcpy(dst, src, size);
    return BM_SUCCESS;
}

/**
 * Выравнивает память по указанной границе
 * @param ptr Указатель на память (адрес перед выравниванием)
 * @param alignment Требуемое выравнивание (должно быть степень 2)
 * @param size Размер для выделения в байтах
 * @return BM_SUCCESS или BM_ERROR
 */
BMResult bm_mem_align(void** ptr, size_t alignment, size_t size) {
    if (!ptr || alignment == 0 || size == 0) {
        bm_set_last_error("bm_mem_align: invalid arguments");
        return BM_ERROR;
    }

    if ((alignment & (alignment - 1)) != 0) {
        bm_set_last_error("bm_mem_align: alignment must be a power of 2");
        return BM_ERROR;
    }

#if defined(_MSC_VER)
    void* p = _aligned_malloc(size, alignment);
    if (!p) {
        bm_set_last_error("bm_mem_align: out of memory");
        return BM_ERROR;
    }
#else
    void* p = NULL;
    if (posix_memalign(&p, alignment, size) != 0) {
        bm_set_last_error("bm_mem_align: out of memory");
        return BM_ERROR;
    }
#endif

    *ptr = p;
    return BM_SUCCESS;
}
