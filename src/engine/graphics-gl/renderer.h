//
// renderer.h
//

#pragma once


// Подключаем:
#include <engine/core/std.h>
#include "shader.h"


// Объявление структур:
typedef struct Renderer Renderer;  // Рендерер.


// Рендерер:
struct Renderer {
    bool initialized;       // Флаг инициализации контекста OpenGL.
    ShaderProgram *shader;  // Дефолтная шейдерная программа.
    void          *camera;  // Текущая активная камера.

    // Функции:

    void (*init)(Renderer *self);           // Инициализация рендерера.
    void (*buffers_flush)(Renderer *self);  // Освобождение буферов.
};


// Создать рендерер:
Renderer* Renderer_create(void);

// Уничтожить рендерер:
void Renderer_destroy(Renderer **rnd);
