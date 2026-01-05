//
// shader.c - Реализует работу с шейдерами.
//


// Подключаем:
#include <engine/core/std.h>
#include <engine/core/math.h>
#include <engine/core/mm.h>
#include <engine/core/array.h>
#include <engine/core/crash.h>
#include "texture.h"
#include "texunit.h"
#include "gl.h"
#include "shader.h"


// Объявление функций:
static void Impl_compile(ShaderProgram *self);
static char* Impl_get_error(ShaderProgram *self);
static void Impl_begin(ShaderProgram *self);
static void Impl_end(ShaderProgram *self);
static int32_t Impl_get_location(ShaderProgram *self, const char* name);
static void Impl_set_bool(ShaderProgram *self, const char* name, bool value);
static void Impl_set_int(ShaderProgram *self, const char* name, int value);
static void Impl_set_float(ShaderProgram *self, const char* name, float value);
static void Impl_set_vec2(ShaderProgram *self, const char* name, Vec2f value);
static void Impl_set_vec3(ShaderProgram *self, const char* name, Vec3f value);
static void Impl_set_vec4(ShaderProgram *self, const char* name, Vec4f value);
static void Impl_set_mat2(ShaderProgram *self, const char* name, mat2 value);
static void Impl_set_mat3(ShaderProgram *self, const char* name, mat3 value);
static void Impl_set_mat4(ShaderProgram *self, const char* name, mat4 value);
static void Impl_set_mat2x3(ShaderProgram *self, const char* name, mat2x3 value);
static void Impl_set_mat3x2(ShaderProgram *self, const char* name, mat3x2 value);
static void Impl_set_mat2x4(ShaderProgram *self, const char* name, mat2x4 value);
static void Impl_set_mat4x2(ShaderProgram *self, const char* name, mat4x2 value);
static void Impl_set_mat3x4(ShaderProgram *self, const char* name, mat3x4 value);
static void Impl_set_mat4x3(ShaderProgram *self, const char* name, mat4x3 value);
static void Impl_set_tex2d(ShaderProgram *self, const char* name, uint32_t tex_id);
static void Impl_set_tex3d(ShaderProgram *self, const char* name, uint32_t tex_id);


static inline ShaderCacheUniformValue* find_cached_uniform(ShaderProgram *self, int loc, ShaderCacheUniformType type) {
    for (size_t i = 0; i < Array_len(self->uniform_values); i++) {
        ShaderCacheUniformValue *item = (ShaderCacheUniformValue*)Array_get(self->uniform_values, i);
        if (item->location == loc && item->type == type) return item;
    }
    return NULL;
}


static inline ShaderCacheSampler* find_cached_sampler(ShaderProgram *self, int32_t location) {
    for (size_t i = 0; i < Array_len(self->sampler_units); i++) {
        ShaderCacheSampler *item = (ShaderCacheSampler*)Array_get(self->sampler_units, i);
        if (item->location == location) return item;
    }
    return NULL;
}


static inline void clear_caches(ShaderProgram *shader, bool delete_arrays) {
    // Освобождаем кэш локаций:
    if (shader->uniform_locations) {
        for (size_t i = 0; i < Array_len(shader->uniform_locations); i++) {
            ShaderCacheUniformLocation *u = Array_get(shader->uniform_locations, i);
            if (u->name) mm_free(u->name);
        }
        if (delete_arrays) { Array_destroy(&shader->uniform_locations); }
    }

    // Освобождаем кэш юниформов:
    if (shader->uniform_values) {
        if (delete_arrays) { Array_destroy(&shader->uniform_values); }
    }

    // Освобождаем кэш юнитов:
    if (shader->sampler_units) {
        for (size_t i = 0; i < Array_len(shader->sampler_units); i++) {
            ShaderCacheSampler *s = Array_get(shader->sampler_units, i);
            if (s->tex_id) {
                TexUnits_unbind(s->tex_id);
            }
        }
        if (delete_arrays) { Array_destroy(&shader->sampler_units); }
    }
}


static inline void set_sampler(ShaderProgram *self, const char* name, uint32_t tex_id, TextureType type) {
    int32_t loc = self->get_location(self, name);
    if (loc < 0) return; // Униформа не найдена.

    // Ищем самплер в кэше:
    ShaderCacheSampler *s = find_cached_sampler(self, loc);

    // Проверяем, есть ли текстура в стеке юнитов (-1 = нет):
    int uid = TexUnits_get_unit_id(tex_id);

    // Если в кэше есть запись, но текстура реально НЕ привязана - нужно перебиндить:
    if (s && uid == -1) {
        // Убрали извне - забываем старый unit, просто перебиндим:
        uint32_t unit_id = TexUnits_bind(tex_id, type);
        s->tex_id = tex_id;
        glUniform1i(loc, unit_id);
        return;
    }

    // Если в кэше есть запись и tex_id совпадает - ничего делать не нужно:
    if (s && s->tex_id == tex_id) {
        return;
    }

    // Если текстура есть в юнитах, но в кэше нет - создаём запись:
    if (!s && uid != -1) {
        ShaderCacheSampler cache = {
            .location = loc,
            .tex_id = tex_id
        };
        Array_push(self->sampler_units, &cache);
        glUniform1i(loc, uid);
        return;
    }

    // Иначе: запись либо новая, либо изменённая, либо отвязанная:
    uint32_t unit_id = TexUnits_bind(tex_id, type);

    if (!s) {
        ShaderCacheSampler cache = {
            .location = loc,
            .tex_id = tex_id
        };
        Array_push(self->sampler_units, &cache);
    } else {
        s->tex_id = tex_id;
    }
    glUniform1i(loc, unit_id);
}


static inline uint32_t compile_shader(ShaderProgram *program, const char* source, GLenum type) {
    if (!source) return 0;
    if (program->error) {
        mm_free(program->error);
        program->error = NULL;
    }

    uint32_t shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    int compiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        int logLength = 0;
        char* log_msg = "No shader source.";
        bool has_source = program->vertex || program->fragment || program->geometry;
        if (has_source) {
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
            log_msg = mm_alloc(logLength);
            glGetShaderInfoLog(shader, logLength, NULL, log_msg);
        }
        // Тип шейдера:
        const char* type_str = (type == GL_VERTEX_SHADER)   ? "VERTEX"   :
                               (type == GL_FRAGMENT_SHADER) ? "FRAGMENT" :
                               (type == GL_GEOMETRY_SHADER) ? "GEOMETRY" : "UNKNOWN";
        // Сколько надо выделить памяти:
        int needed = snprintf(NULL, 0, "ShaderCompileError (%s):\n%s\n", type_str, log_msg);
        program->error = mm_alloc(needed + 1);
        // Форматируем строку:
        sprintf(program->error, "ShaderCompileError (%s):\n%s\n", type_str, log_msg);
        fprintf(stderr, "%s", program->error);
        if (has_source) mm_free(log_msg);
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}


// Регистрируем функции реализации апи для шейдера:
static inline void RegisterAPI(ShaderProgram *shader) {
    shader->compile = Impl_compile;
    shader->get_error = Impl_get_error;
    shader->begin = Impl_begin;
    shader->end = Impl_end;
    shader->get_location = Impl_get_location;
    shader->set_bool = Impl_set_bool;
    shader->set_int = Impl_set_int;
    shader->set_float = Impl_set_float;
    shader->set_vec2 = Impl_set_vec2;
    shader->set_vec3 = Impl_set_vec3;
    shader->set_vec4 = Impl_set_vec4;
    shader->set_mat2 = Impl_set_mat2;
    shader->set_mat3 = Impl_set_mat3;
    shader->set_mat4 = Impl_set_mat4;
    shader->set_mat2x3 = Impl_set_mat2x3;
    shader->set_mat3x2 = Impl_set_mat3x2;
    shader->set_mat2x4 = Impl_set_mat2x4;
    shader->set_mat4x2 = Impl_set_mat4x2;
    shader->set_mat3x4 = Impl_set_mat3x4;
    shader->set_mat4x3 = Impl_set_mat4x3;
    shader->set_tex2d = Impl_set_tex2d;
    shader->set_tex3d = Impl_set_tex3d;
}


// Создать шейдерную программу:
ShaderProgram* ShaderProgram_create(Renderer *renderer, const char *vert, const char *frag, const char *geom) {
    if (!renderer) return NULL;

    // Создаём шейдер:
    ShaderProgram *shader = (ShaderProgram*)mm_alloc(sizeof(ShaderProgram));

    // Заполняем поля:
    shader->vertex = vert;
    shader->fragment = frag;
    shader->geometry = geom;
    shader->error = NULL;
    shader->id = 0;
    shader->renderer = renderer;
    shader->_is_begin_ = false;
    shader->_id_before_begin_ = 0;
    shader->uniform_locations = Array_create(sizeof(ShaderCacheUniformLocation), 128);
    shader->uniform_values = Array_create(sizeof(ShaderCacheUniformValue), 128);
    shader->sampler_units = Array_create(sizeof(ShaderCacheSampler), 128);

    // Регистрируем функции:
    RegisterAPI(shader);
    return shader;
}


// Уничтожить шейдерную программу:
void ShaderProgram_destroy(ShaderProgram **shader) {
    if (!shader || !*shader) return;

    // Освобождаем кэш:
    clear_caches(*shader, true);

    // Удаляем сам шейдер:
    if ((*shader)->id) {
        glDeleteProgram((*shader)->id);
        (*shader)->id = 0;
    }

    // Освобождаем структуру:
    if ((*shader)->error) mm_free((*shader)->error);
    mm_free(*shader);
    *shader = NULL;
}


// Реализация API:


static void Impl_compile(ShaderProgram *self) {
    if (!self) return;

    uint32_t program = glCreateProgram();
    uint32_t shaders[3] = {0};

    if (!program) {
        // Сколько надо выделить памяти:
        int needed = snprintf(NULL, 0, "ShaderCreateError: The OpenGL context has not been created or is inactive.\n");
        self->error = mm_alloc(needed + 1);
        // Форматируем строку:
        sprintf(self->error, "ShaderCreateError: The OpenGL context has not been created or is inactive.\n");
        fprintf(stderr, "%s", self->error);
        return;
    }

    // Компилируем шейдеры:
    if (self->vertex)   shaders[0] = compile_shader(self, self->vertex, GL_VERTEX_SHADER);
    if (self->fragment) shaders[1] = compile_shader(self, self->fragment, GL_FRAGMENT_SHADER);
    if (self->geometry) shaders[2] = compile_shader(self, self->geometry, GL_GEOMETRY_SHADER);

    // Линкуем программу:
    for (int i = 0; i < 3; ++i) {
        if (shaders[i]) glAttachShader(program, shaders[i]);
    }
    glLinkProgram(program);

    // Проверяем статус линковки:
    int linked = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (!linked) {
        int logLength = 0;
        char* log_msg = "No shader source.";
        bool has_source = self->vertex || self->fragment || self->geometry;
        if (has_source) {
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
            log_msg = mm_alloc(logLength);
            glGetProgramInfoLog(program, logLength, NULL, log_msg);
        }
        // Сколько надо выделить памяти:
        int needed = snprintf(NULL, 0, "ShaderLinkingError:\n%s\n", log_msg);
        self->error = mm_alloc(needed + 1);
        // Форматируем строку:
        sprintf(self->error, "ShaderCompileError:\n%s\n", log_msg);
        fprintf(stderr, "%s", self->error);
        crash_print("%s", self->error);
        if (has_source) mm_free(log_msg);
        glDeleteProgram(program);
        return;
    }

    // Удаляем отдельные шейдеры:
    for (int i = 0; i < 3; ++i) {
        if (shaders[i]) {
            glDetachShader(program, shaders[i]);
            glDeleteShader(shaders[i]);
        }
    }
    self->id = program;

    // Очищаем кэш:
    clear_caches(self, false);
}


static char* Impl_get_error(ShaderProgram *self) {
    if (!self) return NULL;
    return self->error;
}


static void Impl_begin(ShaderProgram *self) {
    if (!self) return;
    glGetIntegerv(GL_CURRENT_PROGRAM, &self->_id_before_begin_);
    glUseProgram(self->id);
    self->_is_begin_ = true;
}


static void Impl_end(ShaderProgram *self) {
    if (!self) return;
    glUseProgram((uint32_t)self->_id_before_begin_);
    self->_is_begin_ = false;
}


static int32_t Impl_get_location(ShaderProgram *self, const char* name) {
    if (!self || !self->_is_begin_) return -1;

    // Ищем и возвращаем локацию в кэше:
    for (size_t i = 0; i < Array_len(self->uniform_locations); i++) {
        ShaderCacheUniformLocation *u = (ShaderCacheUniformLocation*)Array_get(self->uniform_locations, i);
        if (strcmp(u->name, name) == 0) {
            return u->location;
        }
    }

    // Иначе получаем локацию и добавляем в кэш:
    int32_t location = glGetUniformLocation(self->id, name);
    if (location == -1) return -1;

    ShaderCacheUniformLocation cache = {
        .name = mm_strdup(name),
        .location = location
    };
    Array_push(self->uniform_locations, &cache);

    return location;
}


static void Impl_set_bool(ShaderProgram *self, const char* name, bool value) {
    if (!self || !self->_is_begin_ || !name) return;
    int32_t loc = self->get_location(self, name);
    if (loc < 0) return; // Униформа не найдена.

    // Ищем униформу в кэше:
    ShaderCacheUniformValue *u = find_cached_uniform(self, loc, SHADERCACHE_UNIFORM_BOOL);

    // Если нашли:
    if (u) {
        if (u->vbool == value) return;  // Если значение не изменилось - выходим.
        u->vbool = value;  // Если значение изменилось - обновляем.
    } else {  // Иначе добавляем новую запись:
        ShaderCacheUniformValue cache = {
            .type = SHADERCACHE_UNIFORM_BOOL,
            .location = loc,
            .vbool = value
        };
        Array_push(self->uniform_values, &cache);
    }
    glUniform1i(loc, (int)value);
}


static void Impl_set_int(ShaderProgram *self, const char* name, int value) {
    if (!self || !self->_is_begin_ || !name) return;
    int32_t loc = self->get_location(self, name);
    if (loc < 0) return; // Униформа не найдена.

    // Ищем униформу в кэше:
    ShaderCacheUniformValue *u = find_cached_uniform(self, loc, SHADERCACHE_UNIFORM_INT);

    // Если нашли:
    if (u) {
        if (u->vint == value) return;  // Если значение не изменилось - выходим.
        u->vint = value;  // Если значение изменилось - обновляем.
    } else {  // Иначе добавляем новую запись:
        ShaderCacheUniformValue cache = {
            .type = SHADERCACHE_UNIFORM_INT,
            .location = loc,
            .vint = value
        };
        Array_push(self->uniform_values, &cache);
    }
    glUniform1i(loc, value);
}


static void Impl_set_float(ShaderProgram *self, const char* name, float value) {
    if (!self || !self->_is_begin_ || !name) return;
    int32_t loc = self->get_location(self, name);
    if (loc < 0) return; // Униформа не найдена.

    // Ищем униформу в кэше:
    ShaderCacheUniformValue *u = find_cached_uniform(self, loc, SHADERCACHE_UNIFORM_FLOAT);

    // Если нашли:
    if (u) {
        if (cmp_float(u->vfloat, value)) return;  // Если значение не изменилось - выходим.
        u->vfloat = value;  // Если значение изменилось - обновляем.
    } else {  // Иначе добавляем новую запись:
        ShaderCacheUniformValue cache = {
            .type = SHADERCACHE_UNIFORM_FLOAT,
            .location = loc,
            .vfloat = value
        };
        Array_push(self->uniform_values, &cache);
    }
    glUniform1f(loc, value);
}


static void Impl_set_vec2(ShaderProgram *self, const char* name, Vec2f value) {
    if (!self || !self->_is_begin_ || !name) return;
    int32_t loc = self->get_location(self, name);
    if (loc < 0) return; // Униформа не найдена.

    // Ищем униформу в кэше:
    ShaderCacheUniformValue *u = find_cached_uniform(self, loc, SHADERCACHE_UNIFORM_VEC2);

    // Если нашли:
    if (u) {
        // Если значение не изменилось - выходим:
        if (cmp_float(u->vec2[0], value.x) && cmp_float(u->vec2[1], value.y)) return;
        // Если значение изменилось - обновляем:
        u->vec2[0] = value.x;
        u->vec2[1] = value.y;
    } else {  // Иначе добавляем новую запись:
        ShaderCacheUniformValue cache = {
            .type = SHADERCACHE_UNIFORM_VEC2,
            .location = loc,
            .vec2[0] = value.x,
            .vec2[1] = value.y
        };
        Array_push(self->uniform_values, &cache);
    }
    glUniform2fv(loc, 1, (float*)&value);
}


static void Impl_set_vec3(ShaderProgram *self, const char* name, Vec3f value) {
    if (!self || !self->_is_begin_ || !name) return;
    int32_t loc = self->get_location(self, name);
    if (loc < 0) return; // Униформа не найдена.

    // Ищем униформу в кэше:
    ShaderCacheUniformValue *u = find_cached_uniform(self, loc, SHADERCACHE_UNIFORM_VEC3);

    // Если нашли:
    if (u) {
        // Если значение не изменилось - выходим:
        if (cmp_float(u->vec3[0], value.x) && cmp_float(u->vec3[1], value.y) && cmp_float(u->vec3[2], value.z)) return;
        // Если значение изменилось - обновляем:
        u->vec3[0] = value.x;
        u->vec3[1] = value.y;
        u->vec3[2] = value.z;
    } else {  // Иначе добавляем новую запись:
        ShaderCacheUniformValue cache = {
            .type = SHADERCACHE_UNIFORM_VEC3,
            .location = loc,
            .vec3[0] = value.x,
            .vec3[1] = value.y,
            .vec3[2] = value.z
        };
        Array_push(self->uniform_values, &cache);
    }
    glUniform3fv(loc, 1, (float*)&value);
}


static void Impl_set_vec4(ShaderProgram *self, const char* name, Vec4f value) {
    if (!self || !self->_is_begin_ || !name) return;
    int32_t loc = self->get_location(self, name);
    if (loc < 0) return; // Униформа не найдена.

    // Ищем униформу в кэше:
    ShaderCacheUniformValue *u = find_cached_uniform(self, loc, SHADERCACHE_UNIFORM_VEC4);

    // Если нашли:
    if (u) {
        // Если значение не изменилось - выходим:
        if (cmp_float(u->vec4[0], value.x) && cmp_float(u->vec4[1], value.y) &&
            cmp_float(u->vec4[2], value.z) && cmp_float(u->vec4[3], value.w)) return;
        // Если значение изменилось - обновляем:
        u->vec4[0] = value.x;
        u->vec4[1] = value.y;
        u->vec4[2] = value.z;
        u->vec4[3] = value.w;
    } else {  // Иначе добавляем новую запись:
        ShaderCacheUniformValue cache = {
            .type = SHADERCACHE_UNIFORM_VEC4,
            .location = loc,
            .vec4[0] = value.x,
            .vec4[1] = value.y,
            .vec4[2] = value.z,
            .vec4[3] = value.w
        };
        Array_push(self->uniform_values, &cache);
    }
    glUniform4fv(loc, 1, (float*)&value);
}


static void Impl_set_mat2(ShaderProgram *self, const char* name, mat2 value) {
    if (!self || !self->_is_begin_ || !name) return;  // Кэширование матриц и массивов слишком дорого и сложно.
    int32_t loc = self->get_location(self, name);
    glUniformMatrix2fv(loc, 1, GL_FALSE, (float*)value);
}


static void Impl_set_mat3(ShaderProgram *self, const char* name, mat3 value) {
    if (!self || !self->_is_begin_ || !name) return;  // Кэширование матриц и массивов слишком дорого и сложно.
    int32_t loc = self->get_location(self, name);
    glUniformMatrix3fv(loc, 1, GL_FALSE, (float*)value);
}


static void Impl_set_mat4(ShaderProgram *self, const char* name, mat4 value) {
    if (!self || !self->_is_begin_ || !name) return;  // Кэширование матриц и массивов слишком дорого и сложно.
    int32_t loc = self->get_location(self, name);
    glUniformMatrix4fv(loc, 1, GL_FALSE, (float*)value);
}


static void Impl_set_mat2x3(ShaderProgram *self, const char* name, mat2x3 value) {
    if (!self || !self->_is_begin_ || !name) return;  // Кэширование матриц и массивов слишком дорого и сложно.
    int32_t loc = self->get_location(self, name);
    glUniformMatrix2x3fv(loc, 1, GL_FALSE, (float*)value);
}


static void Impl_set_mat3x2(ShaderProgram *self, const char* name, mat3x2 value) {
    if (!self || !self->_is_begin_ || !name) return;  // Кэширование матриц и массивов слишком дорого и сложно.
    int32_t loc = self->get_location(self, name);
    glUniformMatrix3x2fv(loc, 1, GL_FALSE, (float*)value);
}


static void Impl_set_mat2x4(ShaderProgram *self, const char* name, mat2x4 value) {
    if (!self || !self->_is_begin_ || !name) return;  // Кэширование матриц и массивов слишком дорого и сложно.
    int32_t loc = self->get_location(self, name);
    glUniformMatrix2x4fv(loc, 1, GL_FALSE, (float*)value);
}


static void Impl_set_mat4x2(ShaderProgram *self, const char* name, mat4x2 value) {
    if (!self || !self->_is_begin_ || !name) return;  // Кэширование матриц и массивов слишком дорого и сложно.
    int32_t loc = self->get_location(self, name);
    glUniformMatrix4x2fv(loc, 1, GL_FALSE, (float*)value);
}


static void Impl_set_mat3x4(ShaderProgram *self, const char* name, mat3x4 value) {
    if (!self || !self->_is_begin_ || !name) return;  // Кэширование матриц и массивов слишком дорого и сложно.
    int32_t loc = self->get_location(self, name);
    glUniformMatrix3x4fv(loc, 1, GL_FALSE, (float*)value);
}


static void Impl_set_mat4x3(ShaderProgram *self, const char* name, mat4x3 value) {
    if (!self || !self->_is_begin_ || !name) return;  // Кэширование матриц и массивов слишком дорого и сложно.
    int32_t loc = self->get_location(self, name);
    glUniformMatrix4x3fv(loc, 1, GL_FALSE, (float*)value);
}


static void Impl_set_tex2d(ShaderProgram *self, const char* name, uint32_t tex_id) {
    if (!self || !self->_is_begin_ || !name || !tex_id) return;
    set_sampler(self, name, tex_id, TEX_TYPE_2D);
}


static void Impl_set_tex3d(ShaderProgram *self, const char* name, uint32_t tex_id) {
    if (!self || !self->_is_begin_ || !name || !tex_id) return;
    set_sampler(self, name, tex_id, TEX_TYPE_3D);
}
