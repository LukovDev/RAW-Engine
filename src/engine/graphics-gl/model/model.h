//
// model.h
//

#pragma once


// Подключаем:
#include <engine/core/std.h>
#include <engine/core/math.h>
#include <engine/core/array.h>
#include "../renderer.h"
#include "mesh.h"


// Объявление структур:
typedef struct Model Model;  // Структура модели.


// Структура модели:
struct Model {
    Vec3d position;  // Позиция модели.
    Vec3d rotation;  // Поворот модели (x=pitch, y=yaw, z=roll).
    Vec3d size;      // Размер модели.
    mat4  model;     // Матрица трансформации модели.
    Array    *meshes;    // Список сеток модели.
    Renderer *renderer;  // Рендерер.

    // Функции:

    void (*update)   (Model *self);              // Обновить матрицу модели.
    void (*render)   (Model *self);              // Отрисовать модель.
    void (*add_mesh) (Model *self, Mesh *mesh);  // Добавить сетку в модель.
};


// Создать модель:
Model* Model_create(Renderer *renderer, Vec3d position, Vec3d rotation, Vec3d size);

// Уничтожить модель:
void Model_destroy(Model **model);
