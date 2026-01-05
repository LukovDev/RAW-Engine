//
// renderer.c - Реализация рендерера для графики.
//


// Подключаем:
#include <engine/core/std.h>
#include <engine/core/mm.h>
#include "buffer_gc.h"
#include "gl.h"
#include "texunit.h"
#include "renderer.h"


// Стандартные шейдеры рендеринга:

// Вершинный шейдер по умолчанию:
static const char* DEFAULT_SHADER_VERT = \
"#version 330 core\n"
"uniform mat4 u_model = mat4(1.0);\n"
"uniform mat4 u_view = mat4(1.0);\n"
"uniform mat4 u_proj = mat4(1.0);\n"
"layout (location = 0) in vec3 a_position;\n"
"layout (location = 1) in vec3 a_normal;\n"
"layout (location = 2) in vec3 a_color;\n"
"layout (location = 3) in vec2 a_texcoord;\n"
"out vec2 v_texcoord;\n"
"out vec3 v_normal;\n"
"out vec3 v_color;\n"
"void main(void) {\n"
"    gl_Position = u_proj * u_view * u_model * vec4(a_position, 1.0);\n"
"    v_texcoord = a_texcoord;\n"
"    v_normal = a_normal;\n"
"    v_color = a_color;\n"
"}\n";

// Фрагментный шейдер по умолчанию:
static const char* DEFAULT_SHADER_FRAG = \
"#version 330 core\n"
"uniform bool u_use_points = false;\n"
"uniform bool u_use_texture;\n"
"uniform bool u_use_normals;\n"
"uniform bool u_use_vcolor;\n"
"uniform vec4 u_color = vec4(1.0);\n"
"uniform sampler2D u_texture;\n"
"in vec2 v_texcoord;\n"
"in vec3 v_normal;\n"
"in vec3 v_color;\n"
"out vec4 FragColor;\n"
"void main(void) {\n"
"    // Если мы используем точки для рисования:\n"
"    if (u_use_points) {\n"
"        vec2 coord = gl_PointCoord*2.0-1.0;\n"
"        if (dot(coord, coord) > 1.0) discard;  // Отбрасываем всё за пределами круга.\n"
"    }\n"
"    // Если мы используем текстуру, рисуем с ней, иначе только цвет:\n"
"    if (u_use_texture) {\n"
"        FragColor = u_color * texture(u_texture, v_texcoord);\n"
"    } else if (u_use_normals) {\n"
"        FragColor = vec4(v_normal.rgb, 1.0);\n"
"    } else if (u_use_vcolor) {\n"
"        FragColor = vec4(v_color.rgb, 1.0);\n"
"    } else {\n"
"        FragColor = u_color;\n"
"    }\n"
"}\n";


// Объявление функций:
static void Impl_init(Renderer *self);
static void Impl_buffers_flush(Renderer *self);


// Регистрируем функции реализации апи:
static void RegisterAPI(Renderer *self) {
    self->init = Impl_init;
    self->buffers_flush = Impl_buffers_flush;
}


// Создать рендерер:
Renderer* Renderer_create() {
    Renderer *rnd = (Renderer*)mm_alloc(sizeof(Renderer));

    // Заполняем поля:
    rnd->initialized = false;
    rnd->shader = NULL;
    rnd->camera = NULL;

    // Создаём шейдер:
    ShaderProgram *shader = ShaderProgram_create(rnd, DEFAULT_SHADER_VERT, DEFAULT_SHADER_FRAG, NULL);
    if (!shader || shader->get_error(shader)) {
        fprintf(stderr, "RENDERER_GL-ERROR: Creating default shader failed: %s\n", shader->error);
        // Самоуничтожение при провале создания шейдера:
        ShaderProgram_destroy(&shader);
        mm_free(rnd);
        return NULL;
    }
    rnd->shader = shader;

    // Регистрируем функции:
    RegisterAPI(rnd);

    return rnd;
}

// Уничтожить рендерер:
void Renderer_destroy(Renderer **rnd) {
    if (!rnd || !*rnd) return;

    // Уничтожение стеков буферов:
    BufferGC_GL_flush();
    BufferGC_GL_destroy();

    // Уничтожение текстурных юнитов:
    TextureUnits_destroy();

    // Освобождаем память шейдера:
    if ((*rnd)->shader) {
        ShaderProgram_destroy(&(*rnd)->shader);
    }

    // Освобождаем память рендерера:
    mm_free(*rnd);
    *rnd = NULL;
}


// Реализация API:


static void Impl_init(Renderer *self) {
    gl_init();  // Инициализируем OpenGL.
    glEnable(GL_BLEND);  // Включаем смешивание цветов.
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  // Устанавливаем режим смешивания.
    glEnable(GL_PROGRAM_POINT_SIZE);  // Разрешаем установку размера точки через шейдер.

    // Делаем нулевой текстурный юнит привязанным к нулевой текстуре:
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Поднимаем флаг инициализации:
    self->initialized = true;

    // Инициализация стеков буферов:
    BufferGC_GL_init();

    // Инициализация текстурных юнитов:
    TextureUnits_init();

    // Компилируем дефолтный шейдер:
    self->shader->compile(self->shader);
}


static void Impl_buffers_flush(Renderer *self) {
    if (!self) return;
    BufferGC_GL_flush();
}
