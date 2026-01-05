//
// vbo.c - Vertex Buffer Object.
//


// Подключаем:
#include <engine/core/std.h>
#include <engine/core/mm.h>
#include "../gl.h"
#include "../buffer_gc.h"
#include "buffers.h"


// Объявление функций:
static void Impl_begin(BufferVBO *self);
static void Impl_end(BufferVBO *self);
static size_t Impl_get_size(BufferVBO *self);
static void Impl_set_data(BufferVBO *self, const void *data, const size_t size, int mode);
static void Impl_set_subdata(BufferVBO *self, const void *data, const size_t offset, const size_t size);


// Создать буфер вершин:
BufferVBO* BufferVBO_create(const void* data, const size_t size, int mode) {
    BufferVBO *vbo = (BufferVBO*)mm_alloc(sizeof(BufferVBO));

    // Заполняем поля:
    vbo->id = 0;
    vbo->_is_begin_ = false;
    vbo->_id_before_begin_ = 0;
    glGenBuffers(1, &vbo->id);

    // Регистрируем API:
    vbo->begin = Impl_begin;
    vbo->end = Impl_end;
    vbo->get_size = Impl_get_size;
    vbo->set_data = Impl_set_data;
    vbo->set_subdata = Impl_set_subdata;

    // Заполняем буфер:
    vbo->begin(vbo);
    vbo->set_data(vbo, data, size, mode);
    vbo->end(vbo);

    return vbo;
}


// Уничтожить буфер вершин:
void BufferVBO_destroy(BufferVBO **vbo) {
    if (!vbo || !*vbo) return;

    // Удаляем сам буфер:
    (*vbo)->end(*vbo);
    BufferGC_GL_push(BGC_GL_VBO, (*vbo)->id);  // Добавляем буфер в стек на уничтожение.
    (*vbo)->_id_before_begin_ = 0;
    (*vbo)->_is_begin_ = false;
    (*vbo)->id = 0;

    mm_free(*vbo);
    *vbo = NULL;
}


// Реализация API:


static void Impl_begin(BufferVBO *self) {
    if (!self || self->_is_begin_ || self->id == 0) return;
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &self->_id_before_begin_);
    glBindBuffer(GL_ARRAY_BUFFER, self->id);
    self->_is_begin_ = true;
}


static void Impl_end(BufferVBO *self) {
    if (!self || !self->_is_begin_) return;
    glBindBuffer(GL_ARRAY_BUFFER, (uint32_t)self->_id_before_begin_);
    self->_is_begin_ = false;
}


static size_t Impl_get_size(BufferVBO *self) {
    if (!self) return 0;
    int buffer_size;
    glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &buffer_size);
    return (size_t)buffer_size;
}


static void Impl_set_data(BufferVBO *self, const void *data, const size_t size, int mode) {
    if (!self) return;
    glBufferData(GL_ARRAY_BUFFER, size, data, mode);
}


static void Impl_set_subdata(BufferVBO *self, const void *data, const size_t offset, const size_t size) {
    if (!self) return;
    glBufferSubData(GL_ARRAY_BUFFER, offset, size, data);
}
