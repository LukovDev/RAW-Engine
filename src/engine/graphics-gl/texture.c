//
// texture.c - Создаёт код для работы с текстурами.
//


// Подключаем:
#include <engine/core/std.h>
#include <engine/core/mm.h>
#include <engine/core/pixmap.h>
#include "buffer_gc.h"
#include "gl.h"
#include "renderer.h"
#include "texture.h"


// Объявление функций:
static void Impl_begin(Texture *self);
static void Impl_end(Texture *self);
static void Impl_load(Texture *self, Pixmap *pixmap, bool use_mipmap);
static void Impl_set_data(Texture *self, const int width, const int height, const void *data, bool use_mipmap,
                          TextureFormat tex_format, TextureFormat data_format, TextureDataType data_type);
static Pixmap* Impl_get_pixmap(Texture *self, int channels);
static void Impl_set_filter(Texture *self, int name, int param);
static void Impl_set_wrap(Texture *self, int axis, int param);
static void Impl_set_linear(Texture *self);
static void Impl_set_pixelized(Texture *self);


// Регистрируем функции реализации апи для текстуры:
static void RegisterAPI(Texture *texture) {
    texture->begin = Impl_begin;
    texture->end = Impl_end;
    texture->load = Impl_load;
    texture->set_data = Impl_set_data;
    texture->get_pixmap = Impl_get_pixmap;
    texture->set_filter = Impl_set_filter;
    texture->set_wrap = Impl_set_wrap;
    texture->set_linear = Impl_set_linear;
    texture->set_pixelized = Impl_set_pixelized;
}


// Создать текстуру:
Texture* Texture_create(Renderer *renderer) {
    if (!renderer) return NULL;

    // Заполняем поля:
    Texture *texture = (Texture*)mm_alloc(sizeof(Texture));
    texture->renderer = renderer;
    texture->id = 0;
    texture->width = 1;
    texture->height = 1;
    texture->channels = 4;
    texture->has_mipmap = false;
    texture->_is_begin_ = false;
    texture->_id_before_begin_ = 0;

    // Регистрируем API:
    RegisterAPI(texture);
    return texture;
}


// Уничтожить текстуру:
void Texture_destroy(Texture **texture) {
    if (!texture || !*texture) return;

    // Удаляем саму текстуру:
    glBindTexture(GL_TEXTURE_2D, 0);
    BufferGC_GL_push(BGC_GL_TBO, (*texture)->id);  // Добавляем буфер в стек на уничтожение.
    (*texture)->_is_begin_ = false;
    (*texture)->id = 0;

    // Освобождаем структуру:
    mm_free(*texture);
    *texture = NULL;
}

// Загрузить текстуру:
void Texture_load(Texture *texture, const char *filepath, bool use_mipmap) {
    Pixmap *img = Pixmap_load(filepath, PIXMAP_RGBA);
    texture->set_data(texture, img->width, img->height, img->data, use_mipmap, TEX_RGBA, TEX_RGBA, TEX_DATA_UBYTE);
    Pixmap_destroy(&img);
}


// Реализация API:


static void Impl_begin(Texture *self) {
    if (!self || self->_is_begin_ || self->id == 0) return;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &self->_id_before_begin_);
    glBindTexture(GL_TEXTURE_2D, self->id);
    self->_is_begin_ = true;
}


static void Impl_end(Texture *self) {
    if (!self || !self->_is_begin_) return;
    glBindTexture(GL_TEXTURE_2D, (uint32_t)self->_id_before_begin_);
    self->_is_begin_ = false;
}


static void Impl_load(Texture *self, Pixmap *pixmap, bool use_mipmap) {
    if (!self || !pixmap) return;

    // Подбираем формат данных:
    TextureFormat tex_format;
    switch (pixmap->channels) {
        case 1:  { tex_format = TEX_RED; break; }
        case 2:  { tex_format = TEX_RG; break; }
        case 3:  { tex_format = TEX_RGB; break; }
        case 4:  { tex_format = TEX_RGBA; break; }
        default: { tex_format = TEX_RGBA; break; }
    }

    // Выделяем память под данные:
    self->set_data(
        self, pixmap->width, pixmap->height, pixmap->data, use_mipmap,
        tex_format, tex_format, TEX_DATA_UBYTE
    );
}


static void Impl_set_data(
    Texture *self, const int width, const int height, const void *data, bool use_mipmap,
    TextureFormat tex_format, TextureFormat data_format, TextureDataType data_type
) {
    if (!self) return;

    self->width = width <= 0 ? 1 : width;
    self->height = height <= 0 ? 1 : height;

    // Если текстура еще не создана, то создаем ее:
    if (self->id == 0) glGenTextures(1, &self->id);
    if (self->id == 0) {  // Если она так и не создалась, то выходим:
        fprintf(stderr, "Texture->set_data: The texture could not be created.\n");
        return;
    }
    self->begin(self);

    // Подбираем формат текстуры:
    int gl_tex_format = GL_RGBA;
    switch (tex_format) {
        case TEX_RED:      { gl_tex_format = GL_RED; break; }
        case TEX_RG:       { gl_tex_format = GL_RG; break; }
        case TEX_RGB:      { gl_tex_format = GL_RGB; break; }
        case TEX_RGBA:     { gl_tex_format = GL_RGBA; break; }
        case TEX_RGB16F:   { gl_tex_format = GL_RGB16F; break; }
        case TEX_RGBA16F:  { gl_tex_format = GL_RGBA16F; break; }
        case TEX_RGB32F:   { gl_tex_format = GL_RGB32F; break; }
        case TEX_RGBA32F:  { gl_tex_format = GL_RGBA32F; break; }
        case TEX_R16F:     { gl_tex_format = GL_R16F; break; }
        case TEX_SRGB:     { gl_tex_format = GL_SRGB8; break; }
        case TEX_SRGBA:    { gl_tex_format = GL_SRGB8_ALPHA8; break; }
        case TEX_BGR:      { gl_tex_format = GL_BGR; break; }
        case TEX_BGRA:     { gl_tex_format = GL_BGRA; break; }
        case TEX_RGBA8:    { gl_tex_format = GL_RGBA8; break; }
        case TEX_DEPTH16:  { gl_tex_format = GL_DEPTH_COMPONENT16; break; }
        case TEX_DEPTH24:  { gl_tex_format = GL_DEPTH_COMPONENT24; break; }
        case TEX_DEPTH32:  { gl_tex_format = GL_DEPTH_COMPONENT32; break; }
        case TEX_DEPTH32F: { gl_tex_format = GL_DEPTH_COMPONENT32F; break; }
        case TEX_DEPTH_COMPONENT: { gl_tex_format = GL_DEPTH_COMPONENT16; break; }
        case TEX_DEPTH_STENCIL:   { gl_tex_format = GL_DEPTH24_STENCIL8; break; }
    }

    // Подбираем формат внешних данных:
    int gl_data_format = GL_RGBA;
    switch (data_format) {
        case TEX_RED:      { gl_data_format = GL_RED; break; }
        case TEX_RG:       { gl_data_format = GL_RG; break; }
        case TEX_RGB:      { gl_data_format = GL_RGB; break; }
        case TEX_RGBA:     { gl_data_format = GL_RGBA; break; }
        case TEX_RGB16F:   { gl_data_format = GL_RGB16F; break; }
        case TEX_RGBA16F:  { gl_data_format = GL_RGBA16F; break; }
        case TEX_RGB32F:   { gl_data_format = GL_RGB32F; break; }
        case TEX_RGBA32F:  { gl_data_format = GL_RGBA32F; break; }
        case TEX_R16F:     { gl_data_format = GL_R16F; break; }
        case TEX_SRGB:     { gl_data_format = GL_SRGB8; break; }
        case TEX_SRGBA:    { gl_data_format = GL_SRGB8_ALPHA8; break; }
        case TEX_BGR:      { gl_data_format = GL_BGR; break; }
        case TEX_BGRA:     { gl_data_format = GL_BGRA; break; }
        case TEX_RGBA8:    { gl_data_format = GL_RGBA8; break; }
        case TEX_DEPTH16:  { gl_data_format = GL_DEPTH_COMPONENT; break; }
        case TEX_DEPTH24:  { gl_data_format = GL_DEPTH_COMPONENT; break; }
        case TEX_DEPTH32:  { gl_tex_format = GL_DEPTH_COMPONENT; break; }
        case TEX_DEPTH32F: { gl_data_format = GL_DEPTH_COMPONENT; break; }
        case TEX_DEPTH_COMPONENT: { gl_data_format = GL_DEPTH_COMPONENT; break; }
        case TEX_DEPTH_STENCIL:   { gl_data_format = GL_DEPTH_STENCIL; break; }
    }

    // Подбираем тип данных:
    int gl_data_type;
    switch (data_type) {
        case TEX_DATA_UBYTE:  { gl_data_type = GL_UNSIGNED_BYTE; break; }
        case TEX_DATA_BYTE:   { gl_data_type = GL_BYTE; break; }
        case TEX_DATA_USHORT: { gl_data_type = GL_UNSIGNED_SHORT; break; }
        case TEX_DATA_SHORT:  { gl_data_type = GL_SHORT; break; }
        case TEX_DATA_UINT:   { gl_data_type = GL_UNSIGNED_INT; break; }
        case TEX_DATA_INT:    { gl_data_type = GL_INT; break; }
        case TEX_DATA_FLOAT:  { gl_data_type = GL_FLOAT; break; }
        default:              { gl_data_type = GL_UNSIGNED_BYTE; break; }
    }

    // Перепроверка:
    if (gl_tex_format == GL_RGB16F || gl_tex_format == GL_RGBA16F) gl_data_type = GL_HALF_FLOAT;
    else if (gl_tex_format == GL_RGB32F || gl_tex_format == GL_RGBA32F) gl_data_type = GL_FLOAT;

    // Загрузка данных текстуры:
    glTexImage2D(GL_TEXTURE_2D, 0, gl_tex_format, self->width, self->height, 0, gl_data_format, gl_data_type, data);

    // Если надо использовать мипмапы, создаём их:
    self->has_mipmap = use_mipmap;
    if (use_mipmap) glGenerateMipmap(GL_TEXTURE_2D);

    // Устанавливаем фильтры по умолчанию:
    self->set_linear(self);
    self->end(self);
}


static Pixmap* Impl_get_pixmap(Texture *self, int channels) {
    if (!self) return NULL;

    // Выделяем память под данные (указатель на блок сохраняется в img ниже):
    unsigned char* data = mm_alloc(self->width * self->height * channels);

    // Подбираем формат данных:
    int gl_data_format;
    switch (channels) {
        case 1:  { gl_data_format = GL_RED; break; }
        case 2:  { gl_data_format = GL_RG; break; }
        case 3:  { gl_data_format = GL_RGB; break; }
        case 4:  { gl_data_format = GL_RGBA; break; }
        default: { gl_data_format = GL_RGBA; break; }
    }
    self->begin(self);
    glGetTexImage(GL_TEXTURE_2D, 0, gl_data_format, GL_UNSIGNED_BYTE, data);
    self->end(self);

    // Создаём изображение:
    Pixmap* pixmap = mm_alloc(sizeof(Pixmap));

    pixmap->width = self->width;
    pixmap->height = self->height;
    pixmap->channels = channels;
    pixmap->from_stbi = false;
    pixmap->data = data;
    return pixmap;  // Не забудьте уничтожить Pixmap!
}


static void Impl_set_filter(Texture *self, int name, int param) {
    if (!self) return;
    self->begin(self);
    glTexParameteri(GL_TEXTURE_2D, name, param);
    self->end(self);
}


static void Impl_set_wrap(Texture *self, int axis, int param) {
    if (!self) return;
    self->begin(self);
    switch (axis) {
        case GL_TEXTURE_WRAP_S: {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, param);
        } break;
        case GL_TEXTURE_WRAP_T: {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, param);
        } break;
        case GL_TEXTURE_WRAP_R: { // Пригодится для 3D текстур.
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, param);
        } break;
        default: {
            // Ничего не делаем.
        } break;
    }
    self->end(self);
}


static void Impl_set_linear(Texture *self) {
    if (!self) return;
    if (!self->has_mipmap) self->set_filter(self, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    else self->set_filter(self, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    self->set_filter(self, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}


static void Impl_set_pixelized(Texture *self) {
    if (!self) return;
    if (!self->has_mipmap) self->set_filter(self, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    else self->set_filter(self, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    self->set_filter(self, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}
