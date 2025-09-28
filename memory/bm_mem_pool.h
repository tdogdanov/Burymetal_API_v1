#ifndef BM_MEM_POOL_H
#define BM_MEM_POOL_H

#include <stddef.h>
#include "bm_types.h"
#include "bm_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------
// Структура пула буферов
// -----------------------------
typedef struct BMBufferPool BMBufferPool;

/**
 * Создание пула буферов для устройства
 * @param device Устройство CPU/GPU
 * @param buffer_size Размер каждого буфера
 * @param count Количество буферов в пуле
 * @return Указатель на пул или NULL при ошибке
 */
BMBufferPool* bm_pool_create(BMDevice* device, size_t buffer_size, size_t count);

/**
 * Уничтожение пула и освобождение всех буферов
 * @param pool Пул буферов
 * @return BM_SUCCESS или BM_ERROR
 */
BMResult bm_pool_destroy(BMBufferPool* pool);

/**
 * Получение свободного буфера из пула
 * @param pool Пул буферов
 * @param out_buffer Указатель для возврата буфера
 * @return BM_SUCCESS или BM_ERROR (если нет свободных)
 */
BMResult bm_pool_acquire(BMBufferPool* pool, BMBuffer** out_buffer);

/**
 * Возврат буфера в пул
 * @param pool Пул буферов
 * @param buffer Буфер для возврата
 * @return BM_SUCCESS или BM_ERROR (если буфер не найден)
 */
BMResult bm_pool_release(BMBufferPool* pool, BMBuffer* buffer);

#ifdef __cplusplus
}
#endif

#endif // BM_MEM_POOL_H
