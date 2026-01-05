//
// model.c - Реализует работу с моделями.
//


// Подключаем:
#include <engine/core/std.h>
#include <engine/core/math.h>
#include <engine/core/array.h>
#include <engine/core/mm.h>
#include "../renderer.h"
#include "../gl.h"
#include "material.h"
#include "mesh.h"
#include "model.h"


// Объявление функций:
static void Impl_update(Model *self);
static void Impl_render(Model *self);
static void Impl_add_mesh(Model *self, Mesh *mesh);


// Создать модель:
Model* Model_create(Renderer *renderer, Vec3d position, Vec3d rotation, Vec3d size) {
    Model *model = (Model*)mm_alloc(sizeof(Model));

    // Заполняем поля:
    model->position = position;
    model->rotation = rotation;
    model->size     = size;
    glm_mat4_identity(model->model);
    model->meshes = Array_create(sizeof(void*), ARRAY_DEFAULT_CAPACITY);
    model->renderer = renderer;

    // Регистрируем API:
    model->update = &Impl_update;
    model->render = &Impl_render;
    model->add_mesh = &Impl_add_mesh;

    // Возвращаем модель:
    return model;
}


// Уничтожить модель:
void Model_destroy(Model **model) {
    if (!model || !*model) return;

    // Проходимся по сеткам и удаляем их:
    for (size_t i=0; i < Array_len((*model)->meshes); i++) {
        Mesh *mesh = Array_get_ptr((*model)->meshes, i);
        Mesh_destroy(&mesh);
    }

    // Уничтожаем массив сеток:
    Array_destroy(&(*model)->meshes);

    // Уничтожить модель:
    mm_free(*model);
    *model = NULL;
}


// Реализация API:


static void Impl_update(Model *self) {
    if (!self) return;
    glm_mat4_identity(self->model);
    glm_scale(self->model, (vec3){self->size.x, self->size.y, self->size.z});
    glm_rotate(self->model, radians(self->rotation.z), (vec3){0, 0, 1});
    glm_rotate(self->model, radians(self->rotation.x), (vec3){1, 0, 0});
    glm_rotate(self->model, radians(self->rotation.y), (vec3){0, 1, 0});
    glm_translate(self->model, (vec3){self->position.x, self->position.y, self->position.z});
}


static void Impl_render(Model *self) {
    if (!self) return;

    // Проходимся по нашим сеткам и рисуем их:
    for (size_t i=0; i < Array_len(self->meshes); i++) {
        Mesh *mesh = Array_get_ptr(self->meshes, i);
        self->renderer->shader->set_bool(self->renderer->shader, "u_use_normals", true);
        self->renderer->shader->set_bool(self->renderer->shader, "u_use_vcolor", false);
        self->renderer->shader->set_bool(self->renderer->shader, "u_use_texture", true);
        mesh->render(mesh, false);
        self->renderer->shader->set_bool(self->renderer->shader, "u_use_normals", false);
        self->renderer->shader->set_bool(self->renderer->shader, "u_use_vcolor", true);
        self->renderer->shader->set_bool(self->renderer->shader, "u_use_texture", false);
        glLineWidth(2.0f);
        mesh->render(mesh, true);
    }
}


static void Impl_add_mesh(Model *self, Mesh *mesh) {
    if (!self || !mesh) return;
    Array_push(self->meshes, &mesh);
}
