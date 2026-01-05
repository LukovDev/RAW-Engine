//
// qbo.c - Query Buffer Object.
//


// Подключаем:
#include <engine/core/std.h>
#include <engine/core/mm.h>
#include "../gl.h"
#include "../buffer_gc.h"
#include "buffers.h"


// Объявление функций:
static void Impl_begin(BufferQBO *self);
static void Impl_end(BufferQBO *self);
static uint32_t Impl_get_primitives(BufferQBO *self);


// Создать буфер отслеживания:
BufferQBO* BufferQBO_create() {
    BufferQBO *qbo = (BufferQBO*)mm_alloc(sizeof(BufferQBO));

    // Заполняем поля:
    qbo->id = 0;
    qbo->_is_begin_ = false;
    glGenQueries(1, &qbo->id);

    // Регистрируем API:
    qbo->begin = Impl_begin;
    qbo->end = Impl_end;
    qbo->get_primitives = Impl_get_primitives;

    return qbo;
}


// Уничтожить буфер отслеживания:
void BufferQBO_destroy(BufferQBO **qbo) {
    if (!qbo || !*qbo) return;

    // Удаляем сам буфер:
    (*qbo)->end(*qbo);
    BufferGC_GL_push(BGC_GL_QBO, (*qbo)->id);  // Добавляем буфер в стек на уничтожение.
    (*qbo)->_is_begin_ = false;
    (*qbo)->id = 0;

    mm_free(*qbo);
    *qbo = NULL;
}


// Реализация API:


static void Impl_begin(BufferQBO *self) {
    if (!self || self->_is_begin_ || self->id == 0) return;
    glBeginQuery(GL_PRIMITIVES_GENERATED, self->id);
    self->_is_begin_ = true;
}


static void Impl_end(BufferQBO *self) {
    if (!self || !self->_is_begin_) return;
    glEndQuery(GL_PRIMITIVES_GENERATED);
    self->_is_begin_ = false;
}


static uint32_t Impl_get_primitives(BufferQBO *self) {
    if (!self) return 0;
    uint32_t result;
    glGetQueryObjectuiv(self->id, GL_QUERY_RESULT, &result);
    return result;
}
