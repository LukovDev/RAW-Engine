//
// obj_parser.c - Простой встроенный парсер OBJ файлов.
//


// Подключаем:
#include <engine/core/std.h>
#include <engine/core/math.h>
#include <engine/core/mm.h>
#include <engine/core/array.h>
#include "../../renderer.h"
#include "../material.h"
#include "../mesh.h"
#include "../model.h"
#include "loader.h"


// Индекс вершины полигона (определение параметров вершины из формата "f p/t/n p/t/n ..."):
typedef struct {
    int p, t, n;  // p - индекс позиции, t - индекс текстурных координат, n - индекс нормали.
} ObjIndex;


// Создать вершину по индексам из полигона:
static inline Vertex make_vertex(ObjIndex idx, Array* positions, Array* normls, Array* texcrd) {
    Vec3d p = *(Vec3d*)Array_get(positions, idx.p);
    Vec3d n = (idx.n >= 0 && idx.n < Array_len(normls)) ? *(Vec3d*)Array_get(normls, idx.n) : (Vec3d){0.0, 0.0, 0.0};
    Vec2d t = (idx.t >= 0 && idx.t < Array_len(texcrd)) ? *(Vec2d*)Array_get(texcrd, idx.t) : (Vec2d){0.0, 0.0};
    return (Vertex){p.x, p.y, p.z, n.x, n.y, n.z, 1.0f, 1.0f, 1.0f, t.x, -t.y};  // X, Y, Z, NX, NY, NZ, R, G, B, U, -V.
}


// Загрузить модели из OBJ-файла:
Array* ModelsLoader_OBJ(Renderer *renderer, const char *filepath) {
    Array *models = Array_create(sizeof(void*), ARRAY_DEFAULT_CAPACITY);

    // Открываем файл:
    FILE* f = fopen(filepath, "r");
    if (!f) return NULL;
    char line[1024];  // Обычно строка в OBJ-файле не превышает 1024 байт.

    // Массивы данных:
    Array *positions = Array_create(sizeof(Vec3d), 128);
    Array *normals = Array_create(sizeof(Vec3d), 128);
    Array *texcoords = Array_create(sizeof(Vec2d), 128);
    Array *vertices  = Array_create(sizeof(Vertex), 256);
    Array *indices   = Array_create(sizeof(uint32_t), 256);

    // Обрабатываем материалы:
    Material *mat = Material_create(NULL, (float[]){1.0f, 1.0f, 1.0f, 1.0f}, NULL);
    Model *model = Model_create(renderer, (Vec3d){0.0f, 0.0f, 0.0f}, (Vec3d){0.0f, 0.0f, 0.0f}, (Vec3d){1.0f, 1.0f, 1.0f});

    // Проходимся по строкам файла:
    while (fgets(line, sizeof(line), f)) {
        // v x y z:
        if (line[0] == 'v' && line[1] == ' ') {
            Vec3d p;
            sscanf(line, "v %lf %lf %lf", &p.x, &p.y, &p.z);
            Array_push(positions, &p);
        }

        // vn x y z:
        else if (line[0] == 'v' && line[1] == 'n') {
            Vec3d n;
            sscanf(line, "vn %lf %lf %lf", &n.x, &n.y, &n.z);
            Array_push(normals, &n);
        }

        // vt u v:
        else if (line[0] == 'v' && line[1] == 't') {
            Vec2d t;
            sscanf(line, "vt %lf %lf", &t.x, &t.y);
            Array_push(texcoords, &t);
        }

        // f a b c:
        else if (line[0] == 'f' && line[1] == ' ') {
            ObjIndex face[64];   // Максимум 64 вершины в полигоне.
            int face_count = 0;  // Количество вершин в полигоне.
            char* token = strtok(line, " ");  // Делим на куски по пробелу.

            // Проходим все вершины полигона:
            while ((token = strtok(NULL, " "))) {  // Получаем очередную вершину.
                ObjIndex idx = {0, -1, -1};  // p всегда 0, t и n = -1 по умолчанию.
                int slash_count = 0;
                for (char* c = token; *c; c++) { if (*c == '/') slash_count++; }

                // Читаем индексы:
                if (slash_count == 0) {  // Только позиции: f 1 2 3:
                    sscanf(token, "%d", &idx.p);
                } else if (slash_count == 1) {  // Позиции + текстуры: f 1/1 2/2 3/3:
                    sscanf(token, "%d/%d", &idx.p, &idx.t);
                } else if (slash_count == 2) {
                    if (strstr(token, "//")) {  // Позиции + нормали: f 1//1 2//2 3//3:
                        sscanf(token, "%d//%d", &idx.p, &idx.n);
                    } else {  // Позиции + текстуры + нормали: f 1/1/1 2/2/2 3/3/3:
                        sscanf(token, "%d/%d/%d", &idx.p, &idx.t, &idx.n);
                    }
                }

                // Индексы в OBJ начинаются с 1, уменьшаем:
                idx.p--; if (idx.t >= 0) idx.t--; if (idx.n >= 0) idx.n--;
                face[face_count++] = idx;  // Сохраняем вершину.
            }

            // Триангуляция (triangle fan) по индексам вершин полигона:
            for (int i = 1; i < face_count - 1; i++) {
                ObjIndex i0 = face[0];
                ObjIndex i1 = face[i];
                ObjIndex i2 = face[i + 1];
                uint32_t base = vertices->len;  // Номер первой вершины треугольника.

                // Создаём вершины треугольника:
                Vertex v0 = make_vertex(i0, positions, normals, texcoords);
                Vertex v1 = make_vertex(i1, positions, normals, texcoords);
                Vertex v2 = make_vertex(i2, positions, normals, texcoords);

                // Добавляем вершины и индексы в массивы:
                Array_push(vertices, &v0);
                Array_push(vertices, &v1);
                Array_push(vertices, &v2);
                Array_push(indices, &(uint32_t){base + 0});
                Array_push(indices, &(uint32_t){base + 1});
                Array_push(indices, &(uint32_t){base + 2});
            }
        }
    }
    fclose(f);

    // Добавляем одну модель:
    model->add_mesh(model, Mesh_create(vertices->data, Array_len(vertices), indices->data, Array_len(indices), false, mat));
    Array_push(models, &model);

    // Удаляем временные массивы:
    Array_destroy(&positions);
    Array_destroy(&normals);
    Array_destroy(&texcoords);
    Array_destroy(&vertices);
    Array_destroy(&indices);

    // Возвращаем список моделей:
    return models;
}
