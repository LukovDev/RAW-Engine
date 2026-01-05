//
// crash.h - Реализация кода обработки ошибок вылета программы.
//

#pragma once


// Инициализация логгера краха:
void crash_logger_init();

// Вывод сообщения в лог-файл и в консоль:
void crash_print(const char *fmt, ...);
