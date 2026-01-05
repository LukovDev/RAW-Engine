//
// texture.h
//

#pragma once


// Подключаем:
#include <engine/core/std.h>
#include <engine/core/pixmap.h>
#include "renderer.h"


// Типы формата данных текстуры и исходников:
typedef enum TextureFormat {
    TEX_RED,     TEX_RG,
    TEX_RGB,     TEX_RGBA,
    TEX_RGB16F,  TEX_RGBA16F,
    TEX_RGB32F,  TEX_RGBA32F,
    TEX_R16F,    TEX_SRGB,
    TEX_SRGBA,   TEX_BGR,
    TEX_BGRA,    TEX_RGBA8,
    TEX_DEPTH16, TEX_DEPTH24,
    TEX_DEPTH32, TEX_DEPTH32F,
    TEX_DEPTH_COMPONENT, TEX_DEPTH_STENCIL,
} TextureFormat;


// Типы данных используемой в текстуре:
typedef enum TextureDataType {
    TEX_DATA_UBYTE,  TEX_DATA_BYTE,
    TEX_DATA_USHORT, TEX_DATA_SHORT,
    TEX_DATA_UINT,   TEX_DATA_INT,
    TEX_DATA_FLOAT,
} TextureDataType;


// Типы текстур:
typedef enum TextureType {
    TEX_TYPE_2D,
    TEX_TYPE_3D,
} TextureType;


// Объявление структур:
typedef struct Texture Texture;  // Текстура 2D.


// Структура текстуры:
struct Texture {
    Renderer *renderer;
    uint32_t id;
    int width;
    int height;
    int channels;
    bool has_mipmap;
    bool _is_begin_;
    int32_t _id_before_begin_;

    // Функции:

    void (*begin) (Texture *self);  // Активация текстуры.
    void (*end)   (Texture *self);  // Деактивация текстуры.
    void (*load)  (Texture *self, Pixmap *pixmap, bool use_mipmap);  // Загрузить текстуру из картинки.

    // Установить данные текстуры:
    void (*set_data) (Texture *self, const int width, const int height, const void *data, bool use_mipmap,
                      TextureFormat tex_format, TextureFormat data_format, TextureDataType data_type);

    Pixmap* (*get_pixmap) (Texture *self, int channels);  // Получить картинку из текстуры.
    void (*set_filter)    (Texture *self, int name, int param);  // Установить фильтрацию текстуры.
    void (*set_wrap)      (Texture *self, int axis, int param);  // Установить повторение текстуры.
    void (*set_linear)    (Texture *self);  // Установить линейную фильтрацию текстуры.
    void (*set_pixelized) (Texture *self);  // Установить пикселизацию текстуры.
};


// Создать текстуру:
Texture* Texture_create(Renderer *renderer);

// Уничтожить текстуру:
void Texture_destroy(Texture **texture);

// Загрузить текстуру:
void Texture_load(Texture *texture, const char *filepath, bool use_mipmap);
