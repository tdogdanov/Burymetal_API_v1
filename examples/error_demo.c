//error_demo.c
#include "bm_utils.h"
#include <stdio.h>

int main() {
    // Устанавливаем уровень логирования
    bm_log_set_level(BM_LOG_LEVEL_DEBUG);

    // Информационное сообщение
    bm_log_info("Запуск программы Burymetal");

    // Симулируем предупреждение
    bm_log_warn("Это предупреждение");

    // Симулируем ошибку
    if (bm_set_last_error("Не удалось выделить память (%d байт)", 1024) != BM_STATUS_OK) {
        bm_log_error("Ошибка установки последней ошибки");
    }

    // Получение последней ошибки
    const char* last_err = bm_get_last_error();
    if (last_err)
        bm_log_info("Последняя ошибка: %s", last_err);
    else
        bm_log_info("Ошибок нет");

    // Дополнительный отладочный вывод
    bm_log_debug("Отладочная информация завершена");

    bm_log_info("Программа завершена успешно");
    return 0;
}
