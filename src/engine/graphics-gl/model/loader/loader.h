//
// loader.h - Загрузчик файлов моделей.
//

#pragma once


// Подключаем:
#include <engine/core/std.h>
#include <engine/core/array.h>
#include "../../renderer.h"


// Загрузить модели из OBJ-файла:
Array* ModelsLoader_OBJ(Renderer *renderer, const char *filepath);
