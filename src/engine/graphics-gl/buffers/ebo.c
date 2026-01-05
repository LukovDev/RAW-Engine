//
// ebo.c - Element Buffer Object.
//


// Подключаем:
#include <engine/core/std.h>
#include <engine/core/mm.h>
#include "../gl.h"
#include "../buffer_gc.h"
#include "buffers.h"


// Объявление функций:
static void Impl_begin(BufferEBO *self);
static void Impl_end(BufferEBO *self);
static int Impl_get_size(BufferEBO *self);
static void Impl_set_data(BufferEBO *self, const void *data, const size_t size, int mode);
static void Impl_set_subdata(BufferEBO *self, const void *data, const size_t offset, const size_t size);


// Создать буфер индексов:
BufferEBO* BufferEBO_create(const void* data, const size_t size, int mode) {
    BufferEBO *ebo = (BufferEBO*)mm_alloc(sizeof(BufferEBO));

    // Заполняем поля:
    ebo->id = 0;
    ebo->_is_begin_ = false;
    ebo->_id_before_begin_ = 0;
    glGenBuffers(1, &ebo->id);

    // Регистрируем API:
    ebo->begin = Impl_begin;
    ebo->end = Impl_end;
    ebo->get_size = Impl_get_size;
    ebo->set_data = Impl_set_data;
    ebo->set_subdata = Impl_set_subdata;

    // Заполняем буфер:
    ebo->begin(ebo);
    ebo->set_data(ebo, data, size, mode);
    ebo->end(ebo);

    return ebo;
}


// Уничтожить буфер индексов:
void BufferEBO_destroy(BufferEBO **ebo) {
    if (!ebo || !*ebo) return;

    // Удаляем сам буфер:
    (*ebo)->end(*ebo);
    BufferGC_GL_push(BGC_GL_EBO, (*ebo)->id);  // Добавляем буфер в стек на уничтожение.
    (*ebo)->_id_before_begin_ = 0;
    (*ebo)->_is_begin_ = false;
    (*ebo)->id = 0;

    mm_free(*ebo);
    *ebo = NULL;
}


// Реализация API:


static void Impl_begin(BufferEBO *self) {
    if (!self || self->_is_begin_ || self->id == 0) return;
    glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &self->_id_before_begin_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self->id);
    self->_is_begin_ = true;
}


static void Impl_end(BufferEBO *self) {
    if (!self || !self->_is_begin_) return;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (uint32_t)self->_id_before_begin_);
    self->_is_begin_ = false;
}


static int Impl_get_size(BufferEBO *self) {
    if (!self) return 0;
    int buffer_size;
    glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &buffer_size);
    return buffer_size;
}


static void Impl_set_data(BufferEBO *self, const void *data, const size_t size, int mode) {
    if (!self) return;
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, mode);
}


static void Impl_set_subdata(BufferEBO *self, const void *data, const size_t offset, const size_t size) {
    if (!self) return;
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, size, data);
}
