//
// window.h
//

#pragma once


// Подключаем:
#include <engine/core/std.h>
#include <engine/core/pixmap.h>
#include "renderer.h"
#include "input.h"


// Объявление структур:
typedef struct Window Window;        // Структура окна.
typedef struct WinConfig WinConfig;  // Конфигурация окна.
typedef struct WinVars WinVars;      // Внутренние переменные окна.


// Типовая структура окна:
struct Window {
    WinConfig *config;    // Конфигурация окна.
    Renderer  *renderer;  // Рендерер.
    Input     *input;     // Ввод.
    WinVars   *vars;      // Указатель на переменные окна.

    // -------------------------------- API окна: --------------------------------

    bool (*create) (Window *self, int gl_major, int gl_minor);  // Вызовите для открытия окна.
    bool (*close)  (Window *self);  // Вызовите для закрытия окна.
    bool (*quit)   (Window *self);  // Вызовите для полного завершения работы всех окон.

    void        (*set_title) (Window *self, const char *title, ...);  // Установить заголовок окна.
    const char* (*get_title) (Window *self);                          // Получить заголовок окна.

    void    (*set_icon) (Window *self, Pixmap *icon);  // Установить иконку окна.
    Pixmap* (*get_icon) (Window *self);                // Получить иконку окна.

    void (*set_size) (Window *self, int width, int height);    // Установить размер окна.
    void (*get_size) (Window *self, int *width, int *height);  // Получить размер окна.

    void (*set_width) (Window *self, int width);  // Установить ширину окна.
    int  (*get_width) (Window *self);             // Получить ширину окна.

    void (*set_height) (Window *self, int height);  // Установить высоту окна.
    int  (*get_height) (Window *self);              // Получить высоту окна.

    void (*get_center) (Window *self, int *x, int *y);  // Получить центр окна.

    void (*set_position) (Window *self, int x, int y);    // Установить позицию окна.
    void (*get_position) (Window *self, int *x, int *y);  // Получить позицию окна.

    void (*set_vsync) (Window *self, bool vsync);  // Установить вертикальную синхронизацию.
    bool (*get_vsync) (Window *self);              // Получить вертикальную синхронизацию.

    void (*set_fps)        (Window *self, int fps);  // Установить фпс окна.
    int  (*get_target_fps) (Window *self);           // Получить установленный фпс окна.

    void (*set_visible) (Window *self, bool visible);  // Установить видимость окна.
    bool (*get_visible) (Window *self);                // Получить видимость окна.

    void (*set_titlebar) (Window *self, bool titlebar);  // Установить видимость заголовка окна.
    bool (*get_titlebar) (Window *self);                 // Получить видимость заголовка окна.

    void (*set_resizable) (Window *self, bool resizable);  // Установить масштабируемость окна.
    bool (*get_resizable) (Window *self);                  // Получить масштабируемость окна.

    void (*set_fullscreen) (Window *self, bool fullscreen);  // Установить полноэкранный режим.
    bool (*get_fullscreen) (Window *self);                   // Получить полноэкранный режим.

    void (*set_min_size) (Window *self, int width, int height);    // Установить мин. размер окна.
    void (*get_min_size) (Window *self, int *width, int *height);  // Получить мин. размер окна.

    void (*set_max_size) (Window *self, int width, int height);    // Установить макс. размер окна.
    void (*get_max_size) (Window *self, int *width, int *height);  // Получить макс. размер окна.

    void (*set_always_top) (Window *self, bool on_top);  // Установить всегда на переднем плане или нет.
    bool (*get_always_top) (Window *self);               // Получить всегда на переднем плане или нет.

    bool (*get_is_focused)   (Window *self);  // Получить фокус окна.
    bool (*get_is_defocused) (Window *self);  // Получить расфокус окна.

    uint32_t (*get_window_display_id) (Window *self);  // Получить айди дисплея в котором это окно.

    bool (*get_display_size) (Window *self, uint32_t id, int *width, int *height);  // Получить размер дисплея.

    void (*maximize) (Window *self);  // Развернуть окно на весь экран.
    void (*minimize) (Window *self);  // Свернуть окно в панель задач.
    void (*restore)  (Window *self);  // Восстановить обычное состояние окна.
    void (*raise)    (Window *self);  // Перенести окно на передний план.

    float  (*get_current_fps) (Window *self);  // Получить текущий фпс.
    double (*get_dtime)       (Window *self);  // Получить дельту времени.
    double (*get_time)        (Window *self);  // Получить время со старта окна.

    void (*clear)   (Window *self, float r, float g, float b);  // Очистить окно.
    void (*display) (Window *self);  // Отрисовка содержимого окна.
};


// Конфигурация окна:
struct WinConfig {
    const char *title;  // Заголовок окна.
    Pixmap *icon;       // Иконка окна.
    bool vsync;         // Вертикальная синхронизация.
    int  fps;           // Количество кадров в секунду.
    bool visible;       // Видимость окна (скрыт/виден).
    bool titlebar;      // Видимость заголовка окна.
    bool resizable;     // Масштабируемость окна.
    bool fullscreen;    // Полноэкранный режим.
    bool always_top;    // Всегда на переднем плане.

    union {
        int size[2];  // Размер окна.
        struct {
            int width;   // Ширина окна.
            int height;  // Высота окна.
        };
    };

    union {
        int position[2];  // Позиция окна.
        struct {
            int x;  // Позиция окна по ширине.
            int y;  // Позиция окна по высоте.
        };
    };

    union {
        int min_size[2];  // Минимальный размер окна.
        struct {
            int min_width;   // Минимальный размер окна по ширине.
            int min_height;  // Минимальный размер окна по высоте.
        };
    };

    union {
        int max_size[2];  // Максимальный размер окна.
        struct {
            int max_width;   // Максимальный размер окна по ширине.
            int max_height;  // Максимальный размер окна по высоте.
        };
    };

    // Основные функции окна (callbacks):
    void (*start)   (Window *self);  // Вызывается после создания окна.
    void (*update)  (Window *self, Input *input, float dtime);  // Вызывается каждый кадр (цикл окна).
    void (*render)  (Window *self, Input *input, float dtime);  // Вызывается каждый кадр (отрисовка окна).
    void (*resize)  (Window *self, int width, int height);      // Вызывается при изменении размера окна.
    void (*show)    (Window *self);  // Вызывается при разворачивании окна.
    void (*hide)    (Window *self);  // Вызывается при сворачивании окна.
    void (*destroy) (Window *self);  // Вызывается при закрытии окна.
};


// Создать конфигурацию окна (аргументы могут быть NULL):
WinConfig* Window_create_config(
    void (*start)   (Window *self),
    void (*update)  (Window *self, Input *input, float dtime),
    void (*render)  (Window *self, Input *input, float dtime),
    void (*resize)  (Window *self, int width, int height),
    void (*show)    (Window *self),
    void (*hide)    (Window *self),
    void (*destroy) (Window *self)
);

// Уничтожить конфигурацию окна:
void Window_destroy_config(WinConfig **config);

// Создать окно:
Window* Window_create(WinConfig *config);

// Уничтожить окно:
void Window_destroy(Window **window);
