//
// camera.h
//

#pragma once


// Подключаем:
#include <engine/core/math.h>
#include <engine/core/std.h>
#include "window.h"


// Объявление структур:
typedef struct Camera2D Camera2D;  // 2D Камера.
typedef struct Camera3D Camera3D;  // 3D Камера.


// Структура 2D камеры:
struct Camera2D {
    Window *window;   // Указатель на окно.
    Vec2d position;   // Позиция камеры.
    float angle;      // Угол наклона камеры.
    float zoom;       // Масштаб камеры.
    float meter;      // Масштаб единицы измерения.
    bool _ui_begin_;  // Отрисовывается ли интерфейс.
    mat4 view;        // Матрица вида.
    mat4 proj;        // Матрица проекции.

    // Можно обратиться как к size.x/y так и к width/height:
    union {
        Vec2i size[2];  // Размер камеры.
        struct {
            int width;   // Ширина камеры.
            int height;  // Высота камеры.
        };
    };

    // Функции:

    void (*update)   (Camera2D *self);  // Обновление камеры.
    void (*resize)   (Camera2D *self, int width, int height);  // Изменение размера камеры.
    void (*ui_begin) (Camera2D *self);  // Начало отрисовки UI.
    void (*ui_end)   (Camera2D *self);  // Конец отрисовки UI.
};


// Структура 3D камеры:
struct Camera3D {
    Window *window;   // Указатель на окно.
    Vec3d position;   // Позиция камеры.
    Vec3d rotation;   // Поворот камеры (x=pitch, y=yaw, z=roll).
    Vec3d size;       // Размер камеры.
    float fov;        // Угол обзора.
    float z_far;      // Дальняя плоскость отсечения.
    float z_near;     // Ближняя плоскость отсечения.
    bool is_ortho;    // Ортографическая камера.
    mat4 view;        // Матрица вида.
    mat4 proj;        // Матрица проекции.
    int width;        // Ширина.
    int height;       // Высота.
    float _oldfov_;   // Старый угол обзора.
    float _oldfar_;   // Старая дальняя плоскость отсечения.
    float _oldnear_;  // Старая ближняя плоскость отсечения.

    // Функции:

    void (*update)  (Camera3D *self);  // Обновление камеры.
    void (*resize)  (Camera3D *self, int width, int height, bool ortho);  // Изменение размера камеры.
    void (*look_at) (Camera3D *self, Vec3d target);  // Посмотреть на указанную точку.
    void (*set_depth_test) (Camera3D *self, bool enabled);  // Установить проверку глубины.
    void (*set_depth_mask) (Camera3D *self, bool enabled);  // Включить или отключить запись глубины.
    void (*set_blending)   (Camera3D *self, bool enabled);  // Включить или отключить смешивание.
    void (*set_cull_faces) (Camera3D *self, bool enabled);  // Установить отсечение граней.
    void (*set_back_face_culling)  (Camera3D *self);  // Отсекать только задние грани.
    void (*set_front_face_culling) (Camera3D *self);  // Отсекать только передние грани.
    void (*set_front_face_onleft)  (Camera3D *self);  // Передняя грань против часовой стрелки (CCW).
    void (*set_front_face_onright) (Camera3D *self);  // Передняя грань по часовой стрелке (CW).
    void (*set_ortho) (Camera3D *self, bool enabled);  // Установить ортографическую проекцию.
    bool (*get_ortho) (Camera3D *self);                // Узнать включена ли ортографическая проекция.
};


// Создать 2D камеру:
Camera2D* Camera2D_create(Window *window, int width, int height, Vec2d position, float angle, float zoom);

// Уничтожить 2D камеру:
void Camera2D_destroy(Camera2D **camera);

// Создать 3D камеру:
Camera3D* Camera3D_create(
    Window *window, int width, int height, Vec3d position, Vec3d rotation,
    Vec3d size, float fov, float z_near, float z_far, bool ortho
);

// Уничтожить 3D камеру:
void Camera3D_destroy(Camera3D **camera);
