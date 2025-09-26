#ifndef BM_UTILS_H
#define BM_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

// --- Логирование ---
#define BM_LOG_LEVEL_ERROR 0
#define BM_LOG_LEVEL_WARN  1
#define BM_LOG_LEVEL_INFO  2
#define BM_LOG_LEVEL_DEBUG 3

void bm_log_set_level(int level);
void bm_log_error(const char* fmt, ...);
void bm_log_warn(const char* fmt, ...);
void bm_log_info(const char* fmt, ...);
void bm_log_debug(const char* fmt, ...);

// --- Ошибки ---
void bm_set_last_error(const char* fmt, ...);
const char* bm_get_last_error(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // BM_UTILS_H
