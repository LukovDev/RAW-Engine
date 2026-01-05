//
// buffers.h
//

#pragma once


// Подключаем:
#include <engine/core/std.h>
#include <engine/core/array.h>


// Типы привязок для кадрового буфера:
typedef enum BufferFBO_Type {
    BUFFER_FBO_COLOR,
    BUFFER_FBO_DEPTH,
    // ...
} BufferFBO_Type;


// Объявление структур:
typedef struct BufferQBO BufferQBO;  // Структура буфера отслеживания.
typedef struct BufferFBO BufferFBO;  // Структура буфера кадра.
typedef struct BufferVBO BufferVBO;  // Структура буфера вершин.
typedef struct BufferEBO BufferEBO;  // Структура буфера индексов.
typedef struct BufferVAO BufferVAO;  // Структура буфера атрибутов.


// Структура буфера отслеживания:
struct BufferQBO {
    uint32_t id;
    bool _is_begin_;

    // Функции:

    void (*begin) (BufferQBO *self);  // Использовать буфер.
    void (*end)   (BufferQBO *self);  // Не использовать буфер.
    uint32_t (*get_primitives) (BufferQBO *self);  // Получить количество отрисованных примитивов.
};


// Структура буфера кадра:
struct BufferFBO {
    int width;
    int height;
    uint32_t id;
    uint32_t rbo_id;
    bool _is_begin_;
    int32_t _id_before_begin_;
    int32_t _rbo_id_before_begin_;
    int32_t _id_before_read_;
    int32_t _id_before_draw_;
    Array *attachments;

    // Функции:

    void (*begin)  (BufferFBO *self);  // Использовать буфер.
    void (*end)    (BufferFBO *self);  // Не использовать буфер.
    void (*clear)  (BufferFBO *self, float r, float g, float b, float a);  // Очистить буфер.
    void (*resize) (BufferFBO *self, int width, int height);  // Изменить размер текстур.
    void (*apply)  (BufferFBO *self);  // Применить массив привязок для записи данных в них.

    // Скопировать цвет и глубину в другой кадровый буфер:
    void (*blit) (BufferFBO *self, uint32_t dest_fbo_id, int x, int y, int width, int height);

    // Скопировать только цвет в другой кадровый буфер:
    void (*blit_color) (BufferFBO *self, uint32_t dest_fbo_id, int x, int y, int width, int height);

    // Скопировать только глубину в другой кадровый буфер:
    void (*blit_depth) (BufferFBO *self, uint32_t dest_fbo_id, int x, int y, int width, int height);

    // Привязать 2D текстуру:
    void (*attach) (BufferFBO *self, BufferFBO_Type type, uint32_t attachment, uint32_t tex_id);
};


// Структура буфера вершин:
struct BufferVBO {
    uint32_t id;
    bool _is_begin_;
    int32_t _id_before_begin_;

    // Функции:

    void   (*begin)    (BufferVBO *self);  // Использовать буфер.
    void   (*end)      (BufferVBO *self);  // Не использовать буфер.
    size_t (*get_size) (BufferVBO *self);  // Получить размер буфера.

    // Установить данные буфера (выделяет новую память и заново всё сохраняет):
    void (*set_data) (BufferVBO *self, const void *data, const size_t size, int mode);

    // Изменить данные буфера (не выделяет новую память а просто изменяет данные):
    void (*set_subdata) (BufferVBO *self, const void *data, const size_t offset, const size_t size);
};


// Структура буфера индексов:
struct BufferEBO {
    uint32_t id;
    bool _is_begin_;
    int32_t _id_before_begin_;

    // Функции:

    void (*begin)    (BufferEBO *self);  // Использовать буфер.
    void (*end)      (BufferEBO *self);  // Не использовать буфер.
    int  (*get_size) (BufferEBO *self);  // Получить размер буфера.

    // Установить данные буфера (выделяет новую память и заново всё сохраняет):
    void (*set_data) (BufferEBO *self, const void *data, const size_t size, int mode);

    // Изменить данные буфера (не выделяет новую память а просто изменяет данные):
    void (*set_subdata) (BufferEBO *self, const void *data, const size_t offset, const size_t size);
};


// Структура буфера атрибутов:
struct BufferVAO {
    uint32_t id;
    bool _is_begin_;

    // Функции:

    void (*begin) (BufferVAO *self);  // Использовать буфер.
    void (*end)   (BufferVAO *self);  // Не использовать буфер.

    // Установить атрибуты вершин:
    void (*attrib_pointer) (
        BufferVAO *self, int loc, size_t count, int type, bool normalize, size_t stride, size_t offset
    );
};


// Создать буфер отслеживания:
BufferQBO* BufferQBO_create();
// Уничтожить буфер отслеживания:
void BufferQBO_destroy(BufferQBO **qbo);


// Создать буфер кадра:
BufferFBO* BufferFBO_create(int width, int height);
// Уничтожить буфер кадра:
void BufferFBO_destroy(BufferFBO **fbo);


// Создать буфер вершин:
BufferVBO* BufferVBO_create(const void* data, const size_t size, int mode);
// Уничтожить буфер вершин:
void BufferVBO_destroy(BufferVBO **vbo);


// Создать буфер индексов:
BufferEBO* BufferEBO_create(const void* data, const size_t size, int mode);
// Уничтожить буфер индексов:
void BufferEBO_destroy(BufferEBO **vbo);


// Создать буфер атрибутов:
BufferVAO* BufferVAO_create();
// Уничтожить буфер атрибутов:
void BufferVAO_destroy(BufferVAO **vao);
