//
// mesh.h
//

#pragma once


// Подключаем:
#include <engine/core/std.h>
#include "../buffers/buffers.h"
#include "material.h"


// Объявление структур:
typedef struct Vertex Vertex;  // Структура вершины.
typedef struct Mesh Mesh;      // Структура одной сетки модели.


// Структура вершины:
struct Vertex {
    float px, py, pz;  // Позиция вершины (loc=0, vec3 a_position).
    float nx, ny, nz;  // Нормаль вершины (loc=1, vec3 a_normal).
    float r, g, b;     // Цвет вершины          (loc=2, vec3 a_color).
    float u, v;        // Текстурные координаты (loc=3, vec2 a_texcoord).
};


// Структура сетки:
struct Mesh {
    BufferVAO *vao;  // Атрибуты вершин.
    BufferVBO *vbo;  // Буфер вершин.
    BufferEBO *ebo;  // Буфер индексов.
    uint32_t index_count;  // Количество индексов.
    bool is_dynamic;       // Динамическая ли сетка.
    Material *material;    // Материал сетки.

    // Функции:

    void (*render) (Mesh *self, bool wireframe);  // Простой способ отрисовать сетку через forward rendering.
};


// Создать сетку:
Mesh* Mesh_create(
    const Vertex* vertices,
    uint32_t vertex_count,
    const uint32_t* indices,
    uint32_t index_count,
    bool is_dynamic,
    Material* material
);

// Уничтожить сетку:
void Mesh_destroy(Mesh **mesh);
