//
// main.c - Основной файл программы.
//


// Подключаем:
#include <engine/core/core.h>
#include <engine/graphics-gl/graphics.h>


void engine_init() {
    printf("Start initialization.\n");
    core_init();
    printf("Core initialized.\n");

    printf("Engine start.\n\n");
}


void print_before_free() {
    printf("(Before free) MM used: %g kb (%zu b). Blocks allocated: %zu. Absolute: %zu b. BlockHeaderSize: %zu b.\n",
            mm_get_used_size_kb(), mm_get_used_size(), mm_get_total_allocated_blocks(), mm_get_absolute_used_size(),
            mm_get_block_header_size());
}


void print_after_free() {
    printf("(After free) MM used: %g kb (%zu b). Blocks allocated: %zu. Absolute: %zu b. BlockHeaderSize: %zu b.\n",
            mm_get_used_size_kb(), mm_get_used_size(), mm_get_total_allocated_blocks(), mm_get_absolute_used_size(),
            mm_get_block_header_size());
    if (mm_get_used_size() > 0) printf("Memory leak!\n");
}


Texture *tex;
Texture *tex2;
ShaderProgram *shader;
Camera3D *camera3d;
CameraController3D *controller3d;

Mesh *mesh;
Material *mat;
Model *model;

// Вершины треугольника (X, Y)
static const float f = 1.0f;
// static const Vertex triangle_vertices[] = {
//     {-f, -f, 0,    0, 0, 0,    1, 1, 1,    0, 1},
//     {+f, -f, 0,    0, 0, 0,    1, 1, 1,    1, 1},
//     {+f, +f, 0,    0, 0, 0,    1, 1, 1,    1, 0},
//     {+f, +f, 0,    0, 0, 0,    1, 1, 1,    1, 0},
//     {-f, +f, 0,    0, 0, 0,    1, 1, 1,    0, 0},
//     {-f, -f, 0,    0, 0, 0,    1, 1, 1,    0, 1}
// }
static const Vertex quad_vertices[] = {
    {-f, -f, 0,  0,0,0,  1,1,1,  0,1}, // 0
    {+f, -f, 0,  0,0,0,  1,1,1,  1,1}, // 1
    {+f, +f, 0,  0,0,0,  1,1,1,  1,0}, // 2
    {-f, +f, 0,  0,0,0,  1,1,1,  0,0}  // 3
};
static const uint32_t quad_indices[] = {
    0, 1, 2,
    2, 3, 0
};


// Вызывается после создания окна:
void start(Window *self) {
    printf("Start called.\n");
    self->set_title(self, "RAW Engine Experimental");
    self->set_fps(self, 60);
    self->set_vsync(self, false);

    Pixmap *icon = Pixmap_load("data/icons/icon.png", PIXMAP_RGBA);
    self->set_icon(self, icon);
    Pixmap_destroy(&icon);

    tex = Texture_create(self->renderer);
    Texture_load(tex, NULL, true);
    tex->set_pixelized(tex);

    tex2 = Texture_create(self->renderer);
    Texture_load(tex2, "data/textures/gradient_uv_checker.png", false);
    // tex2->set_pixelized(tex2);

    printf("loading shaders... ");
    const char *vert1 = Files_load("data/shaders/default.vert", "r");
    const char *frag1 = Files_load("data/shaders/default.frag", "r");
    if (vert1 && frag1) printf("Done.\n");
    else printf("Failed.\n");
    shader = ShaderProgram_create(self->renderer, vert1, frag1, NULL);
    shader->compile(shader);
    mm_free((char*)vert1);
    mm_free((char*)frag1);

    camera3d = Camera3D_create(
        self, self->get_width(self), self->get_height(self),
        (Vec3d){0.0f, 0.0f, 0.0f},
        (Vec3d){0.0f, 0.0f, 0.0f},
        (Vec3d){1.0f, 1.0f, 1.0f},
        90.0f,
        0.01f, 10.0f,
        false
    );
    camera3d->set_cull_faces(camera3d, false);
    controller3d = CameraController3D_create(self, camera3d, 0.1f, 1.0f, 5.0f, 25.0f, 0.75f, false);

    mat = Material_create(NULL, (float[]){1.0f, 1.0f, 1.0f, 1.0f}, tex2);
    mesh = Mesh_create(quad_vertices, sizeof(quad_vertices) / sizeof(Vertex), quad_indices, sizeof(quad_indices) / sizeof(uint32_t), false, mat);

    model = Model_create(self->renderer, (Vec3d){0.0f, 0.0f, 0.0f}, (Vec3d){0.0f, 0.0f, 0.0f}, (Vec3d){1.0f, 1.0f, 1.0f});
    model->add_mesh(model, mesh);

    Array *arr = ModelsLoader_OBJ(self->renderer, "data/models/micro_scenes/micro_scene_002.obj");
    if (!arr) printf("Failed to load obj file.\n");
    else {
        Model_destroy(&model);
        Array_print(arr, stdout, ARRAY_PRINT_PTR);
        model = Array_get_ptr(arr, 0);
    }
    Array_destroy(&arr);
}


// Вызывается каждый кадр (цикл окна):
void update(Window *self, Input *input, float dtime) {
    if (false) {
        if (is_windows()) system("cls");
        if (is_linux() || is_macos()) system("clear");
        printf("FPS: %g\n", self->get_current_fps(self));
        printf("MM used: %g kb (%zu b). Blocks allocated: %zu\n",
                mm_get_used_size_kb(), mm_get_used_size(), mm_get_total_allocated_blocks());
    }

    if (self->get_is_focused(self)) {
        self->set_fps(self, 60.0f);
    }
    if (self->get_is_defocused(self)) {
        self->set_fps(self, 10.0f);
    }

    if (input->get_key_down(self)[K_1]) camera3d->set_back_face_culling(camera3d);
    if (input->get_key_down(self)[K_2]) camera3d->set_front_face_culling(camera3d);

    controller3d->update(controller3d, dtime, false);
    camera3d->update(camera3d);

    model->update(model);
}


// Вызывается каждый кадр (отрисовка окна):
void render(Window *self, Input *input, float dtime) {
    self->clear(self, 0.1f, 0.3f, 0.5f);

    float t = self->get_time(self);
    float r = 0.5 + 0.5 * cosf(t + 0.0);
    float g = 0.5 + 0.5 * cosf(t + 2.0);
    float b = 0.5 + 0.5 * cosf(t + 4.0);
    Renderer *rnd = self->renderer;

    //

    rnd->shader->set_tex2d(rnd->shader, "u_texture", mat->albedo->id);
    rnd->shader->set_bool(rnd->shader, "u_use_texture", true);

    rnd->shader->set_mat4(rnd->shader, "u_model", model->model);
    model->render(model);

    self->display(self);
}


// Вызывается при изменении размера окна:
void resize(Window *self, int width, int height) {
    printf("Resize called.\n");
    //camera->resize(camera, width, height);
    camera3d->resize(camera3d, width, height, camera3d->get_ortho(camera3d));
}


// Вызывается при разворачивании окна:
void show(Window *self) {
    printf("Show called.\n");
}


// Вызывается при сворачивании окна:
void hide(Window *self) {
    printf("Hide called.\n");
}


// Вызывается при закрытии окна:
void destroy(Window *self) {
    printf("Destroy called.\n");

    // Уничтожение:
    print_before_free();

    Model_destroy(&model);
    Material_destroy(&mat);

    Texture_destroy(&tex);
    Texture_destroy(&tex2);
    ShaderProgram_destroy(&shader);
    Camera3D_destroy(&camera3d);
    CameraController3D_destroy(&controller3d);
}


// Основная функция:
int main(int argc, char* argv[]) {
    engine_init();

    WinConfig *config = Window_create_config(start, update, render, resize, show, hide, destroy);
    // config->width = 256;
    // config->height = 256;
    Window *window = Window_create(config);
    if (!window->create(window, 3, 3)) {
        crash_print("Window creation failed.\n");
    }

    Window_destroy_config(&config);
    Window_destroy(&window);

    print_after_free();
    printf("\nEngine stop.\n");
    return 0;
}
