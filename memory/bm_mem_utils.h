#ifndef BM_MEM_UTILS_H
#define BM_MEM_UTILS_H

#include <stddef.h>
#include "bm_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------
// Вспомогательные функции для работы с памятью
// -----------------------------

/**
 * Обнуляет память
 * @param ptr Указатель на память
 * @param size Размер в байтах
 * @return BM_SUCCESS или BM_ERROR
 */
BMResult bm_mem_zero(void* ptr, size_t size);

/**
 * Копирует память
 * @param dst Указатель на целевую память
 * @param src Указатель на источник
 * @param size Размер в байтах
 * @return BM_SUCCESS или BM_ERROR
 */
BMResult bm_mem_copy(void* dst, const void* src, size_t size);

/**
 * Выравнивает память по указанной границе
 * @param ptr Указатель на память (адрес перед выравниванием)
 * @param alignment Требуемое выравнивание (должно быть степень 2)
 * @param size Размер для выделения в байтах
 * @return BM_SUCCESS или BM_ERROR
 */
BMResult bm_mem_align(void** ptr, size_t alignment, size_t size);

#ifdef __cplusplus
}
#endif

#endif // BM_MEM_UTILS_H
