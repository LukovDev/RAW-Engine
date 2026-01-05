//
// shader.h
//

#pragma once


// Подключаем:
#include <engine/core/std.h>
#include <engine/core/math.h>
#include <engine/core/array.h>


// Виды значений юниформов для кэша:
typedef enum ShaderCacheUniformType {
    SHADERCACHE_UNIFORM_BOOL,
    SHADERCACHE_UNIFORM_INT,
    SHADERCACHE_UNIFORM_FLOAT,
    SHADERCACHE_UNIFORM_VEC2,
    SHADERCACHE_UNIFORM_VEC3,
    SHADERCACHE_UNIFORM_VEC4,
} ShaderCacheUniformType;


// Объявление структур:
typedef struct ShaderProgram ShaderProgram;  // Шейдерная программа.
typedef struct ShaderCacheUniformLocation ShaderCacheUniformLocation;
typedef struct ShaderCacheUniformValue ShaderCacheUniformValue;
typedef struct ShaderCacheSampler ShaderCacheSampler;
typedef struct Renderer Renderer;


// Единица кэша локаций юниформов:
struct ShaderCacheUniformLocation {
    char *name;        // Имя юниформа.
    int32_t location;  // Позиция в шейдере.
};


// Единица кэша значений юниформов:
struct ShaderCacheUniformValue {
    ShaderCacheUniformType type;  // Тип значения.
    int32_t location;  // Позиция в шейдере.
    union {  // Данные:
        bool vbool;
        int32_t vint;
        float vfloat;
        float vec2[2];
        float vec3[3];
        float vec4[4];
    };
};


// Единица кэша сэмплеров:
struct ShaderCacheSampler {
    int32_t  location;
    uint32_t tex_id;
};


// Структура шейдера:
struct ShaderProgram {
    const char* vertex;
    const char* fragment;
    const char* geometry;
    char* error;
    uint32_t id;
    Renderer *renderer;
    bool _is_begin_;
    int32_t _id_before_begin_;

    // Динамические списки для кэша параметров шейдера:
    Array *uniform_locations;  // Кэш позиций uniform.
    Array *uniform_values;     // Кэш значений uniform (всё кроме массивов и матриц).
    Array *sampler_units;      // Кэш привязки текстурных юнитов к названиям униформов.

    // Функции:

    void  (*compile)   (ShaderProgram *self);  // Компиляция шейдеров в программу.
    char* (*get_error) (ShaderProgram *self);  // Получить ошибку компиляции или линковки.
    void  (*begin)     (ShaderProgram *self);  // Активация программы.
    void  (*end)       (ShaderProgram *self);  // Деактивация программы.

    int32_t (*get_location) (ShaderProgram *self, const char* name);  // Получить локацию переменной.

    void (*set_bool)  (ShaderProgram *self, const char* name, bool value);   // Установить значение bool.
    void (*set_int)   (ShaderProgram *self, const char* name, int value);    // Установить значение int.
    void (*set_float) (ShaderProgram *self, const char* name, float value);  // Установить значение float.

    void (*set_vec2) (ShaderProgram *self, const char* name, Vec2f value);  // Установить значение vec2.
    void (*set_vec3) (ShaderProgram *self, const char* name, Vec3f value);  // Установить значение vec3.
    void (*set_vec4) (ShaderProgram *self, const char* name, Vec4f value);  // Установить значение vec4.

    void (*set_mat2)   (ShaderProgram *self, const char* name, mat2   value);  // Установить значение mat2.
    void (*set_mat3)   (ShaderProgram *self, const char* name, mat3   value);  // Установить значение mat3.
    void (*set_mat4)   (ShaderProgram *self, const char* name, mat4   value);  // Установить значение mat4.
    void (*set_mat2x3) (ShaderProgram *self, const char* name, mat2x3 value);  // Установить значение mat2x3.
    void (*set_mat3x2) (ShaderProgram *self, const char* name, mat3x2 value);  // Установить значение mat3x2.
    void (*set_mat2x4) (ShaderProgram *self, const char* name, mat2x4 value);  // Установить значение mat2x4.
    void (*set_mat4x2) (ShaderProgram *self, const char* name, mat4x2 value);  // Установить значение mat4x2.
    void (*set_mat3x4) (ShaderProgram *self, const char* name, mat3x4 value);  // Установить значение mat3x4.
    void (*set_mat4x3) (ShaderProgram *self, const char* name, mat4x3 value);  // Установить значение mat4x3.

    void (*set_tex2d) (ShaderProgram *self, const char* name, uint32_t tex_id);  // Установить 2D текстуру.
    void (*set_tex3d) (ShaderProgram *self, const char* name, uint32_t tex_id);  // Установить 3D текстуру.
};


// Создать шейдерную программу:
ShaderProgram* ShaderProgram_create(Renderer *renderer, const char *vert, const char *frag, const char *geom);

// Уничтожить шейдерную программу:
void ShaderProgram_destroy(ShaderProgram **shader);
