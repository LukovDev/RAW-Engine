//
// material.c - Реализует работу с материалами.
//


// Подключаем:
#include <engine/core/std.h>
#include <engine/core/mm.h>
#include "../shader.h"
#include "../texture.h"
#include "material.h"


// Создать материал:
Material* Material_create(
    ShaderProgram *shader,
    float color[4],
    Texture *albedo
) {
    Material *mat = (Material*)mm_alloc(sizeof(Material));

    // Заполняем поля:
    mat->shader = shader;
    mat->color[0] = color[0];
    mat->color[1] = color[1];
    mat->color[2] = color[2];
    mat->color[3] = color[3];
    mat->albedo = albedo;

    return mat;
}


// Уничтожить материал:
void Material_destroy(Material **mat) {
    if (!mat || !*mat) return;
    mm_free(*mat);
    *mat = NULL;
}
