#ifndef BM_UTILS_H
#define BM_UTILS_H

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

// --- Коды ошибок ---
typedef enum {
    BM_OK = 0,
    BM_ERROR_NOMEM,
    BM_ERROR_INVALID_ARG,
    BM_ERROR_INTERNAL,
} BMResult;

const char* bm_result_string(BMResult code);

// --- Логирование ---
typedef enum {
    BM_LOG_ERROR = 0,
    BM_LOG_WARN,
    BM_LOG_INFO,
    BM_LOG_DEBUG
} BMLogLevel;

// Устанавливает уровень логирования
BMResult bm_log_set_level(BMLogLevel level);

// Логирование сообщений (потокобезопасно)
BMResult bm_log(BMLogLevel level, const char* fmt, ...);

// --- Потокобезопасный last_error ---
BMResult bm_set_last_error(const char* fmt, ...);
const char* bm_get_last_error(void); // возвращает thread-local строку

#ifdef __cplusplus
} // extern "C"
#endif

#endif // BM_UTILS_H
