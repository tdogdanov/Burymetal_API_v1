#include "bm_mem_pool.h"
#include "bm_mem_alloc.h"
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

// Структура пула
struct BMBufferPool {
    BMDevice* device;
    size_t buffer_size;
    size_t count;
    BMBuffer** buffers;
    int* used; // флаг занятости буфера
#ifdef _WIN32
    CRITICAL_SECTION lock;
#else
    pthread_mutex_t lock;
#endif
};

// -----------------------------
// Вспомогательные функции
// -----------------------------

static void pool_lock(BMBufferPool* pool) {
#ifdef _WIN32
    EnterCriticalSection(&pool->lock);
#else
    pthread_mutex_lock(&pool->lock);
#endif
}

static void pool_unlock(BMBufferPool* pool) {
#ifdef _WIN32
    LeaveCriticalSection(&pool->lock);
#else
    pthread_mutex_unlock(&pool->lock);
#endif
}

// -----------------------------
// Создание/удаление пула
// -----------------------------

BMBufferPool* bm_pool_create(BMDevice* device, size_t buffer_size, size_t count) {
    if (!device || buffer_size == 0 || count == 0) {
        bm_set_last_error("bm_pool_create: invalid arguments");
        return NULL;
    }

    BMBufferPool* pool = (BMBufferPool*)malloc(sizeof(BMBufferPool));
    if (!pool) {
        bm_set_last_error("bm_pool_create: out of memory");
        return NULL;
    }

    pool->device = device;
    pool->buffer_size = buffer_size;
    pool->count = count;
    pool->buffers = (BMBuffer**)malloc(sizeof(BMBuffer*) * count);
    pool->used = (int*)malloc(sizeof(int) * count);
    if (!pool->buffers || !pool->used) {
        free(pool->buffers);
        free(pool->used);
        free(pool);
        bm_set_last_error("bm_pool_create: out of memory");
        return NULL;
    }

    memset(pool->used, 0, sizeof(int) * count);

    for (size_t i = 0; i < count; ++i) {
        void* data_ptr = NULL;
        if (device->type == BM_CPU) {
            if (bm_cpu_alloc(buffer_size, &data_ptr) != BM_SUCCESS) {
                // Очистка уже выделенных буферов
                for (size_t j = 0; j < i; ++j)
                    bm_cpu_free(pool->buffers[j]->data);
                free(pool->buffers);
                free(pool->used);
                free(pool);
                return NULL;
            }
        } else {
            if (bm_gpu_alloc(device, buffer_size, &data_ptr) != BM_SUCCESS) {
                for (size_t j = 0; j < i; ++j)
                    bm_gpu_free(device, pool->buffers[j]->data);
                free(pool->buffers);
                free(pool->used);
                free(pool);
                return NULL;
            }
        }

        BMBuffer* buf = (BMBuffer*)malloc(sizeof(BMBuffer));
        buf->device = device;
        buf->size = buffer_size;
        buf->data = data_ptr;
        pool->buffers[i] = buf;
    }

#ifdef _WIN32
    InitializeCriticalSection(&pool->lock);
#else
    pthread_mutex_init(&pool->lock, NULL);
#endif

    return pool;
}

BMResult bm_pool_destroy(BMBufferPool* pool) {
    if (!pool) return BM_ERROR;

#ifdef _WIN32
    DeleteCriticalSection(&pool->lock);
#else
    pthread_mutex_destroy(&pool->lock);
#endif

    for (size_t i = 0; i < pool->count; ++i) {
        if (pool->buffers[i]) {
            if (pool->device->type == BM_CPU)
                bm_cpu_free(pool->buffers[i]->data);
            else
                bm_gpu_free(pool->device, pool->buffers[i]->data);
            free(pool->buffers[i]);
        }
    }

    free(pool->buffers);
    free(pool->used);
    free(pool);
    return BM_SUCCESS;
}

// -----------------------------
// Получение/возврат буфера
// -----------------------------

BMResult bm_pool_acquire(BMBufferPool* pool, BMBuffer** out_buffer) {
    if (!pool || !out_buffer) {
        bm_set_last_error("bm_pool_acquire: invalid arguments");
        return BM_ERROR;
    }

    pool_lock(pool);

    for (size_t i = 0; i < pool->count; ++i) {
        if (!pool->used[i]) {
            pool->used[i] = 1;
            *out_buffer = pool->buffers[i];
            pool_unlock(pool);
            return BM_SUCCESS;
        }
    }

    pool_unlock(pool);
    bm_set_last_error("bm_pool_acquire: no free buffers");
    return BM_ERROR;
}

BMResult bm_pool_release(BMBufferPool* pool, BMBuffer* buffer) {
    if (!pool || !buffer) {
        bm_set_last_error("bm_pool_release: invalid arguments");
        return BM_ERROR;
    }

    pool_lock(pool);

    for (size_t i = 0; i < pool->count; ++i) {
        if (pool->buffers[i] == buffer) {
            pool->used[i] = 0;
            pool_unlock(pool);
            return BM_SUCCESS;
        }
    }

    pool_unlock(pool);
    bm_set_last_error("bm_pool_release: buffer not found in pool");
    return BM_ERROR;
}
