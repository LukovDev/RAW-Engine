//
// graphics.h - Модуль графики. Объединяет весь функционал модуля графики.
//
// Этот модуль рассчитан на OpenGL+SDL3 графику. Если вы хотите использовать другой
// графический API, то вам нужно будет переписать этот модуль с сохранением его API,
// воизбежание переписывания основного кода проекта.
//

#pragma once

#ifdef __cplusplus
extern "C" {
#endif


// Подключаем:
// #include "buffer_gc.h" - НЕ подключаем, т.к. это внутренний функционал модуля графики.
#include "buffers/buffers.h"
#include "controllers/controllers.h"
#include "model/material.h"
#include "model/mesh.h"
#include "model/model.h"
#include "model/loader/loader.h"
#include "camera.h"
#include "gl.h"
#include "input.h"
#include "renderer.h"
#include "shader.h"
#include "texture.h"
#include "texunit.h"
#include "window.h"


#ifdef __cplusplus
}
#endif
