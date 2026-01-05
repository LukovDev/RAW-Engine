//
// material.h
//

#pragma once


// Подключаем:
#include <engine/core/std.h>
#include "../shader.h"
#include "../texture.h"


// Объявление структур:
typedef struct Material Material;  // Структура материала.


// Структура материала:
struct Material {
    ShaderProgram *shader;  // Шейдер материала (NULL=шейдер по умолчанию).
    float color[4];         // RGBA цвет.
    Texture *albedo;        // Текстура основного цвета.
};


// Создать материал:
Material* Material_create(
    ShaderProgram *shader,
    float color[4],
    Texture *albedo
);


// Уничтожить материал:
void Material_destroy(Material **mat);
