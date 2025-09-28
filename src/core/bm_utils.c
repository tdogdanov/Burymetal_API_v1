// bm_utils.c
#include "bm_utils.h"
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#include <synchapi.h>
#define THREAD_LOCAL __declspec(thread)
#else
#include <pthread.h>
#define THREAD_LOCAL __thread
#endif

// ----------------------------------------
// Логирование
// ----------------------------------------

typedef enum {
    BM_LOG_ERROR,
    BM_LOG_WARN,
    BM_LOG_INFO,
    BM_LOG_DEBUG
} BMLogLevel;

static BMLogLevel current_level = BM_LOG_INFO;

// Файл для логов
static FILE* log_file = NULL;

// Мьютекс для потокобезопасного логирования
#ifdef _WIN32
static CRITICAL_SECTION log_mutex;
static int log_mutex_initialized = 0;
#else
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

// Потокобезопасный last_error
static THREAD_LOCAL char last_error[256] = {0};

// Цвета для консоли
#ifdef _WIN32
#define COLOR_ERROR   FOREGROUND_RED | FOREGROUND_INTENSITY
#define COLOR_WARN    FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY
#define COLOR_INFO    FOREGROUND_GREEN | FOREGROUND_INTENSITY
#define COLOR_DEBUG   FOREGROUND_BLUE | FOREGROUND_INTENSITY
#else
#define COLOR_ERROR   "\033[31m"
#define COLOR_WARN    "\033[33m"
#define COLOR_INFO    "\033[32m"
#define COLOR_DEBUG   "\033[34m"
#define COLOR_RESET   "\033[0m"
#endif

static void lock_mutex() {
#ifdef _WIN32
    if (!log_mutex_initialized) { InitializeCriticalSection(&log_mutex); log_mutex_initialized = 1; }
    EnterCriticalSection(&log_mutex);
#else
    pthread_mutex_lock(&log_mutex);
#endif
}

static void unlock_mutex() {
#ifdef _WIN32
    LeaveCriticalSection(&log_mutex);
#else
    pthread_mutex_unlock(&log_mutex);
#endif
}

// Внутренняя функция логирования
static void bm_vlog(BMLogLevel level, const char* fmt, va_list args) {
    if (level > current_level) return;

    const char* level_str =
        (level == BM_LOG_ERROR) ? "ERROR" :
        (level == BM_LOG_WARN)  ? "WARN"  :
        (level == BM_LOG_INFO)  ? "INFO"  : "DEBUG";

#ifdef _WIN32
    WORD color;
    switch(level) {
        case BM_LOG_ERROR: color = COLOR_ERROR; break;
        case BM_LOG_WARN:  color = COLOR_WARN;  break;
        case BM_LOG_INFO:  color = COLOR_INFO;  break;
        case BM_LOG_DEBUG: color = COLOR_DEBUG; break;
    }
    HANDLE hConsole = GetStdHandle(STD_ERROR_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    WORD old_color = csbi.wAttributes;
    SetConsoleTextAttribute(hConsole, color);
#else
    const char* color =
        (level == BM_LOG_ERROR) ? COLOR_ERROR :
        (level == BM_LOG_WARN)  ? COLOR_WARN :
        (level == BM_LOG_INFO)  ? COLOR_INFO : COLOR_DEBUG;
#endif

    time_t now = time(NULL);
    struct tm tbuf;
#ifdef _WIN32
    localtime_s(&tbuf, &now);
#else
    localtime_r(&now, &tbuf);
#endif
    char ts[32];
    strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", &tbuf);

    lock_mutex();

#ifdef _WIN32
    fprintf(stderr, "[%s] [%s] ", ts, level_str);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
#else
    fprintf(stderr, "%s[%s] [%s]%s ", color, ts, level_str, COLOR_RESET);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
#endif

    if (log_file) {
        fprintf(log_file, "[%s] [%s] ", ts, level_str);
        vfprintf(log_file, fmt, args);
        fprintf(log_file, "\n");
        fflush(log_file);
    }

#ifdef _WIN32
    SetConsoleTextAttribute(hConsole, old_color);
#endif

    unlock_mutex();
}

// ----------------------------------------
// Интерфейс логирования
// ----------------------------------------

void bm_log_set_level(int level) { current_level = (BMLogLevel)level; }

void bm_log_set_file(const char* path) {
    lock_mutex();
    if (log_file) fclose(log_file);
    log_file = path ? fopen(path, "a") : NULL;
    if (!log_file && path) {
        fprintf(stderr, "WARNING: не удалось открыть файл для логов: %s\n", path);
    }
    unlock_mutex();
}

void bm_log_error(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    bm_vlog(BM_LOG_ERROR, fmt, args);
    va_end(args);
}

void bm_log_warn(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    bm_vlog(BM_LOG_WARN, fmt, args);
    va_end(args);
}

void bm_log_info(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    bm_vlog(BM_LOG_INFO, fmt, args);
    va_end(args);
}

void bm_log_debug(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    bm_vlog(BM_LOG_DEBUG, fmt, args);
    va_end(args);
}

// ----------------------------------------
// Система ошибок
// ----------------------------------------

void bm_set_last_error(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
#ifdef _WIN32
    _vsnprintf_s(last_error, sizeof(last_error), _TRUNCATE, fmt, args);
#else
    vsnprintf(last_error, sizeof(last_error), fmt, args);
#endif
    va_end(args);
    // Автоматически логируем
    bm_log_error("%s", last_error);
}

const char* bm_get_last_error(void) {
    return last_error[0] ? last_error : "OK";
}
