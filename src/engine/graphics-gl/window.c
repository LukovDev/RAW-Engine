//
// window.c - Реализация работы окна в си. Основано на SDL3 + OpenGL.
//


// Подключаем:
#include <SDL3/SDL.h>
#include <engine/core/std.h>
#include <engine/core/mm.h>
#include <engine/core/pixmap.h>
#include <engine/core/crash.h>
#include "input.h"
#include "renderer.h"
#include "gl.h"
#include "window.h"


// Структура переменных окна:
struct WinVars {
    SDL_Window *window;
    SDL_GLContext context;
    char title[256];  // 255 символа для заголовка окна + 1 для '\0'.
    uint64_t perf_freq;
    double start_time;
    double dtime;
    double dtime_old;
    bool create_failed;
    bool running;
    bool focused;
    bool defocused;
    bool closing;
};


// Объявление функций:
// Вспомогательные функции:
static void RegisterAPI(Window *window);
static void MainLoop(Window *self, WinConfig *cfg);
static void Closing_stage(Window *self);
static Input_Scancode Convert_scancode(SDL_Scancode scancode);
static void Impl_set_mouse_pos(Window *self, int x, int y);
static void Impl_set_mouse_visible(Window *self, bool visible);
// Основной функционал окна:
static bool Impl_create(Window* self, int gl_major, int gl_minor);
static bool Impl_close(Window *self);
static bool Impl_quit(Window *self);
static void Impl_set_title(Window *self, const char *title, ...);
static const char* Impl_get_title(Window *self);
static void Impl_set_icon(Window *self, Pixmap *icon);
static Pixmap* Impl_get_icon(Window *self);
static void Impl_set_size(Window *self, int width, int height);
static void Impl_get_size(Window *self, int *width, int *height);
static void Impl_set_width(Window *self, int width);
static int Impl_get_width(Window *self);
static void Impl_set_height(Window *self, int height);
static int Impl_get_height(Window *self);
static void Impl_get_center(Window *self, int *x, int *y);
static void Impl_set_position(Window *self, int x, int y);
static void Impl_get_position(Window *self, int *x, int *y);
static void Impl_set_vsync(Window *self, bool vsync);
static bool Impl_get_vsync(Window *self);
static void Impl_set_fps(Window *self, int fps);
static int Impl_get_target_fps(Window *self);
static void Impl_set_visible(Window *self, bool visible);
static bool Impl_get_visible(Window *self);
static void Impl_set_titlebar(Window *self, bool titlebar);
static bool Impl_get_titlebar(Window *self);
static void Impl_set_resizable(Window *self, bool resizable);
static bool Impl_get_resizable(Window *self);
static void Impl_set_fullscreen(Window *self, bool fullscreen);
static bool Impl_get_fullscreen(Window *self);
static void Impl_set_min_size(Window *self, int width, int height);
static void Impl_get_min_size(Window *self, int *width, int *height);
static void Impl_set_max_size(Window *self, int width, int height);
static void Impl_get_max_size(Window *self, int *width, int *height);
static void Impl_set_always_top(Window *self, bool on_top);
static bool Impl_get_always_top(Window *self);
static bool Impl_get_is_focused(Window *self);
static bool Impl_get_is_defocused(Window *self);
static uint32_t Impl_get_window_display_id(Window *self);
static bool Impl_get_display_size(Window *self, uint32_t id, int *width, int *height);
static void Impl_maximize(Window *self);
static void Impl_minimize(Window *self);
static void Impl_restore(Window *self);
static void Impl_raise(Window *self);
static float Impl_get_current_fps(Window *self);
static double Impl_get_dtime(Window *self);
static double Impl_get_time(Window *self);
static void Impl_clear(Window *self, float r, float g, float b);
static void Impl_display(Window *self);


// Регистрируем функции реализации апи:
static void RegisterAPI(Window *window) {
    window->create = Impl_create;
    window->close = Impl_close;
    window->quit = Impl_quit;
    window->set_title = Impl_set_title;
    window->get_title = Impl_get_title;
    window->set_icon = Impl_set_icon;
    window->get_icon = Impl_get_icon;
    window->set_size = Impl_set_size;
    window->get_size = Impl_get_size;
    window->set_width = Impl_set_width;
    window->get_width = Impl_get_width;
    window->set_height = Impl_set_height;
    window->get_height = Impl_get_height;
    window->get_center = Impl_get_center;
    window->set_position = Impl_set_position;
    window->get_position = Impl_get_position;
    window->set_vsync = Impl_set_vsync;
    window->get_vsync = Impl_get_vsync;
    window->set_fps = Impl_set_fps;
    window->get_target_fps = Impl_get_target_fps;
    window->set_visible = Impl_set_visible;
    window->get_visible = Impl_get_visible;
    window->set_titlebar = Impl_set_titlebar;
    window->get_titlebar = Impl_get_titlebar;
    window->set_resizable = Impl_set_resizable;
    window->get_resizable = Impl_get_resizable;
    window->set_fullscreen = Impl_set_fullscreen;
    window->get_fullscreen = Impl_get_fullscreen;
    window->set_min_size = Impl_set_min_size;
    window->get_min_size = Impl_get_min_size;
    window->set_max_size = Impl_set_max_size;
    window->get_max_size = Impl_get_max_size;
    window->set_always_top = Impl_set_always_top;
    window->get_always_top = Impl_get_always_top;
    window->get_is_focused = Impl_get_is_focused;
    window->get_is_defocused = Impl_get_is_defocused;
    window->get_window_display_id = Impl_get_window_display_id;
    window->get_display_size = Impl_get_display_size;
    window->maximize = Impl_maximize;
    window->minimize = Impl_minimize;
    window->restore = Impl_restore;
    window->raise = Impl_raise;
    window->get_current_fps = Impl_get_current_fps;
    window->get_dtime = Impl_get_dtime;
    window->get_time = Impl_get_time;
    window->clear = Impl_clear;
    window->display = Impl_display;
}


// Создать конфигурацию окна (аргументы могут быть NULL):
WinConfig* Window_create_config(
    void (*start)   (Window *self),
    void (*update)  (Window *self, Input *input, float dtime),
    void (*render)  (Window *self, Input *input, float dtime),
    void (*resize)  (Window *self, int width, int height),
    void (*show)    (Window *self),
    void (*hide)    (Window *self),
    void (*destroy) (Window *self)
) {
    WinConfig* config = (WinConfig*)mm_alloc(sizeof(WinConfig));

    // Заполняем поля (значениями по умолчанию):
    config->title = "Untitled";
    config->icon = NULL;
    config->width = 960;
    config->height = 540;
    config->x = -1;  // Выравнивание по центру экрана.
    config->y = -1;  // Выравнивание по центру экрана.
    config->vsync = false;
    config->fps = 60;
    config->visible = true;
    config->titlebar = true;
    config->resizable = true;
    config->fullscreen = false;
    config->always_top = false;
    config->min_width = 0;
    config->min_height = 0;
    config->max_width = 0;
    config->max_height = 0;

    // Заполняем функции:
    config->start = start;
    config->update = update;
    config->render = render;
    config->resize = resize;
    config->show = show;
    config->hide = hide;
    config->destroy = destroy;

    // Возвращаем конфигурацию:
    return config;
}


// Уничтожить конфигурацию окна:
void Window_destroy_config(WinConfig **config) {
    if (!config || !*config) return;
    if ((*config)->icon != NULL) Pixmap_destroy(&(*config)->icon);
    mm_free(*config);
    *config = NULL;
}


// Создать окно:
Window* Window_create(WinConfig *config) {
    Window *window = (Window*)mm_alloc(sizeof(Window));

    // Создаём локальные переменные окна:
    WinVars *vars = (WinVars*)mm_calloc(1, sizeof(WinVars));

    // Создаём систему ввода:
    Input *input = Input_create(Impl_set_mouse_pos, Impl_set_mouse_visible);

    // Сохраняем указатели:
    window->config = config;
    window->renderer = NULL;  // Создаём рендерер при создании окна.
    window->input = input;
    window->vars = vars;

    // Регистрируем функции:
    RegisterAPI(window);

    // Возвращаем окно:
    return window;
}


// Уничтожить окно:
void Window_destroy(Window **window) {
    if (!window || !*window) return;
    WinVars *vars = (*window)->vars;

    // Вызываем закрытие окна, если оно ещё не закрыто:
    if (vars->window && !vars->create_failed) {
        (*window)->close(*window);
        (*window)->quit(*window);
    }

    // Удаляем рендерер:
    if ((*window)->renderer) Renderer_destroy(&(*window)->renderer);

    // Освобождаем память системы ввода:
    if ((*window)->input) Input_destroy(&(*window)->input);

    // Освобождаем память глобальных переменных:
    if (vars) {
        mm_free(vars);
        vars = NULL;
    }

    // Освободить память окна:
    mm_free(*window);
    *window = NULL;
}


// Главный цикл окна (ядро окна. Запускается из Impl_create):
static void MainLoop(Window *self, WinConfig *cfg) {
    // Получаем глобальные переменные окна:
    WinVars *vars = self->vars;
    if (!vars || !vars->window) return;

    // Вызываем старт:
    if (cfg->start) cfg->start(self);

    // Основной цикл окна:
    vars->running = true;
    while (vars->running) {
        // Настраиваем переменные:
        double frame_start = self->get_time(self);
        vars->focused = false;
        vars->defocused = false;

        // Проверяем чтобы дельта времени не была равна нулю. Иначе используем прошлую дельту времени:
        if (vars->dtime > 0.0) { vars->dtime_old = vars->dtime; }
        else { vars->dtime = vars->dtime_old; }

        // Получаем копию указателя на систему ввода:
        Input *input = self->input;

        // Сброс состояний клавиатуры и мыши:
        input->mouse->rel = (Vec2i){0, 0};
        input->mouse->wheel = (Vec2i){0, 0};
        memset(input->mouse->down, 0, input->mouse->max_keys * sizeof(bool));
        memset(input->mouse->up,   0, input->mouse->max_keys * sizeof(bool));
        memset(input->keyboard->down, 0, input->keyboard->max_keys * sizeof(bool));
        memset(input->keyboard->up,   0, input->keyboard->max_keys * sizeof(bool));

        // Обрабатываем события:
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                // Если окно хотят закрыть:
                case SDL_EVENT_QUIT: {
                    self->close(self);
                } break;

                // Если размер окна изменился:
                case SDL_EVENT_WINDOW_RESIZED: {
                    if (event.window.data1 > 0 && event.window.data2 > 0 && cfg->resize && vars->context) {
                        cfg->resize(self, event.window.data1, event.window.data2);
                    }
                } break;

                // Окно развернули:
                case SDL_EVENT_WINDOW_RESTORED: {
                    if (cfg->show) cfg->show(self);
                } break;

                // Окно свернули:
                case SDL_EVENT_WINDOW_MINIMIZED: {
                    if (cfg->hide) cfg->hide(self);
                } break;

                // Окно стало активным:
                case SDL_EVENT_WINDOW_FOCUS_GAINED: {
                    vars->focused = true;
                } break;

                // Окно потеряло фокус:
                case SDL_EVENT_WINDOW_FOCUS_LOST: {
                    vars->defocused = true;
                } break;

                // Обработка ввода:

                // Если мышь зашла в окно:
                case SDL_EVENT_WINDOW_MOUSE_ENTER: {
                    input->mouse->focused = true;
                } break;

                // Если мышь покинула окно:
                case SDL_EVENT_WINDOW_MOUSE_LEAVE: {
                    input->mouse->focused = false;
                } break;

                // Если мышь передвинулась:
                case SDL_EVENT_MOUSE_MOTION: {
                    input->mouse->rel.x = event.motion.xrel;
                    input->mouse->rel.y = event.motion.yrel;
                    input->mouse->pos.x = event.motion.x;
                    input->mouse->pos.y = event.motion.y;
                } break;

                // Если колёсико мыши провернулось:
                case SDL_EVENT_MOUSE_WHEEL: {
                    input->mouse->wheel.x = event.wheel.x;
                    input->mouse->wheel.y = event.wheel.y;
                } break;

                // Если нажимают кнопку мыши:
                case SDL_EVENT_MOUSE_BUTTON_DOWN: {
                    if (event.button.button < input->mouse->max_keys) {
                        input->mouse->pressed[event.button.button - 1] = true;
                        input->mouse->down[event.button.button - 1] = true;
                    }
                } break;

                // Если отпускают кнопку мыши:
                case SDL_EVENT_MOUSE_BUTTON_UP: {
                    if (event.button.button < input->mouse->max_keys) {
                        input->mouse->pressed[event.button.button - 1] = false;
                        input->mouse->up[event.button.button - 1] = true;
                    }
                } break;

                // Если нажимают кнопку на клавиатуре:
                case SDL_EVENT_KEY_DOWN: {
                    Input_Scancode scancode = Convert_scancode(event.key.scancode);
                    if (scancode < input->keyboard->max_keys) {
                        if (!input->keyboard->pressed[scancode]) {
                            input->keyboard->pressed[scancode] = true;
                            input->keyboard->down[scancode] = true;
                        }
                    }
                } break;

                // Если отпускают кнопку на клавиатуре:
                case SDL_EVENT_KEY_UP: {
                    Input_Scancode scancode = Convert_scancode(event.key.scancode);
                    if (scancode < input->keyboard->max_keys) {
                        input->keyboard->pressed[scancode] = false;
                        input->keyboard->up[scancode] = true;
                    }
                } break;
            }
        }

        // Обработка основных функций (обновление и отрисовка):
        if (cfg->update) cfg->update(self, self->input, self->get_dtime(self));
        if (cfg->render) cfg->render(self, self->input, self->get_dtime(self));

        // Очищаем все буфера (массивное удаление всех буферов за раз):
        self->renderer->buffers_flush(self->renderer);

        // Проверяем что окно хотят закрыть:
        if (vars->closing) {
            Closing_stage(self);
            return;
        }

        // Делаем задержку между кадрами:
        if (!self->config->vsync && cfg->fps > 0) {
            double target = 1.0f / (float)cfg->fps;
            double elapsed;  // Сколько прошло времени с начала кадра в секундах.
            do {
                elapsed = self->get_time(self) - frame_start;
                if (target - elapsed > 0.002) {  // По 2 мс ожидание:
                    SDL_Delay(1);  // Задержка 1 мс + затраты на тайминги ос примерно >1 мс.
                }
            } while (elapsed < target);  // Цикл ожидания тайминга до нужного времени.
        } else if (!self->config->vsync && cfg->fps <= 0) { /* Никакой задержки. */ }
        else { SDL_Delay(0); }  // При vsync можно не задерживать - SDL будет синхронизировать кадры.

        // Получаем дельту времени (время кадра или же время обработки одного цикла окна):
        vars->dtime = self->get_time(self) - frame_start;
    }

    // Закрываем окно:
    self->close(self);
    if (vars->closing) Closing_stage(self);
}


// Этап закрытия окна:
static void Closing_stage(Window *self) {
    if (!self || !self->config) return;
    WinVars *vars = self->vars;
    if (!vars || !vars->window) return;

    // Вызываем уничтожение:
    if (self->config->destroy) {
        self->config->destroy(self);
    }

    // Уничтожаем контекст рендеринга:
    SDL_GL_DestroyContext(vars->context);
    vars->context = NULL;

    // Уничтожаем окно:
    SDL_DestroyWindow(vars->window);
    memset(vars, 0, sizeof(WinVars));
}


// Реализация API:


static bool Impl_create(Window *self, int gl_major, int gl_minor) {
    if (!self || !self->config) return false;
    WinConfig *cfg = self->config;
    WinVars *vars = self->vars;
    if (!vars) return false;

    // Минимальная версия OpenGL:
    if (gl_major < 3) gl_major = 3;
    if (gl_minor < 3) gl_minor = 3;

    // Инициализируем SDL:
    if (SDL_Init(SDL_INIT_VIDEO) == false) {
        crash_print("SDL_Init Error: %s\n", SDL_GetError());
        vars->create_failed = true;
        return false;
    }

    // Флаги окна:
    SDL_WindowFlags flags = SDL_WINDOW_HIDDEN;

    // Настраиваем профиль OpenGL:
    uint32_t profile = SDL_GL_CONTEXT_PROFILE_CORE;  // По умолчанию.
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, gl_major);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, gl_minor);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, profile);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);  // Включаем двоибуферизацию.
    flags |= SDL_WINDOW_OPENGL;

    // Инициализируем окно:
    SDL_Window *window = SDL_CreateWindow(cfg->title, cfg->width, cfg->height, flags);
    if (!window) {
        crash_print("SDL_CreateWindow Error: %s\n", SDL_GetError());
        vars->create_failed = true;
        SDL_Quit();
        return false;
    }

    // Создаём контекст:
    vars->context = SDL_GL_CreateContext(window);
    if (!vars->context) {
        crash_print("SDL_GL_CreateContext Error: %s\n", SDL_GetError());
        vars->create_failed = true;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return false;
    }
    SDL_GL_MakeCurrent(window, vars->context);  // Делаем контекст активным.
    SDL_GL_SetSwapInterval(0);  // По умолчанию, отключаем VSync.

    // Создаём рендерер:
    Renderer *rnd = Renderer_create();
    self->renderer = rnd;  // Указываем рендерер в окне.

    // Инициализируем рендерер:
    rnd->init(rnd);

    // Устанавливаем значения в глобальные переменные:
    vars->window = window;
    vars->perf_freq = SDL_GetPerformanceFrequency();
    vars->start_time = self->get_time(self);
    vars->dtime = cfg->fps <= 0 ? 1.0/60.0 : 1.0/cfg->fps;
    vars->dtime_old = vars->dtime;
    vars->closing = false;

    // Настройка окна:
    self->set_vsync(self, cfg->vsync);
    self->set_fps(self, cfg->fps);
    self->set_position(self, cfg->x, cfg->y);
    self->set_titlebar(self, cfg->titlebar);
    self->set_resizable(self, cfg->resizable);
    self->set_fullscreen(self, cfg->fullscreen);
    self->set_min_size(self, cfg->min_width, cfg->min_height);
    self->set_max_size(self, cfg->max_width, cfg->max_height);
    self->set_always_top(self, cfg->always_top);
    self->set_visible(self, cfg->visible);  // Применяем видимость только после применения других настроек.

    // Запускаем главный цикл:
    MainLoop(self, cfg);
    return true;
}


static bool Impl_close(Window *self) {
    if (!self || !self->config) return false;
    WinVars *vars = self->vars;
    if (!vars || !vars->window) return false;
    vars->closing = true;
    return true;
}


static bool Impl_quit(Window *self) {
    if (!self) return false;
    WinVars *vars = self->vars;

    // Останавливаем все окна и закрываем SDL:
    if (vars && vars->window) Impl_close(self);
    SDL_Quit();
    return true;
}


static void Impl_set_title(Window *self, const char *title, ...) {
    if (!self || !self->config) return;
    WinVars *vars = self->vars;
    if (!vars || !vars->window) return;

    va_list args;
    va_start(args, title);
    vsnprintf(vars->title, sizeof(vars->title), title, args);
    va_end(args);
    SDL_SetWindowTitle(vars->window, vars->title);
    self->config->title = vars->title;
}


static const char* Impl_get_title(Window *self) {
    if (!self) return NULL;
    WinVars *vars = self->vars;
    if (!vars || !vars->window) return NULL;
    return SDL_GetWindowTitle(vars->window);
}


static void Impl_set_icon(Window *self, Pixmap *icon) {
    if (!self) return;
    WinVars *vars = self->vars;
    if (!vars || !vars->window) return;

    if (self->config->icon) Pixmap_destroy(&self->config->icon);
    self->config->icon = Pixmap_copy(icon);

    if (!icon) {
        SDL_Surface *empty_icon = SDL_CreateSurface(1, 1, SDL_PIXELFORMAT_RGBA32);
        SDL_memset(empty_icon->pixels, 0, empty_icon->h * empty_icon->pitch);
        if (empty_icon) {
            SDL_SetWindowIcon(vars->window, empty_icon);
            SDL_DestroySurface(empty_icon);
        }
    } else {
        SDL_Surface *sdl_icon = SDL_CreateSurfaceFrom(
            icon->width, icon->height, SDL_PIXELFORMAT_RGBA32,
            icon->data, icon->width * icon->channels);
        if (icon) {
            SDL_SetWindowIcon(vars->window, sdl_icon);
            SDL_DestroySurface(sdl_icon);
        }
    }
}


static Pixmap* Impl_get_icon(Window *self) {
    if (!self || !self->config) return NULL;
    return self->config->icon;
}


static void Impl_set_size(Window *self, int width, int height) {
    if (!self || !self->config) return;
    WinVars *vars = self->vars;
    if (!vars || !vars->window) return;
    SDL_SetWindowSize(vars->window, width, height);
    self->config->width = width;
    self->config->height = height;
}


static void Impl_get_size(Window *self, int *width, int *height) {
    if (!self) return;
    WinVars *vars = self->vars;
    if (!vars || !vars->window) return;
    SDL_GetWindowSize(vars->window, width, height);
}


static void Impl_set_width(Window *self, int width) {
    if (!self || !self->config) return;
    WinVars *vars = self->vars;
    if (!vars || !vars->window) return;
    SDL_SetWindowSize(vars->window, width, self->config->height);
    self->config->width = width;
}


static int Impl_get_width(Window *self) {
    if (!self) return 0;
    WinVars *vars = self->vars;
    if (!vars || !vars->window) return 0;
    int width;
    SDL_GetWindowSize(vars->window, &width, NULL);
    return width;
}


static void Impl_set_height(Window *self, int height) {
    if (!self || !self->config) return;
    WinVars *vars = self->vars;
    if (!vars || !vars->window) return;
    SDL_SetWindowSize(vars->window, self->config->width, height);
    self->config->height = height;
}


static int Impl_get_height(Window *self) {
    if (!self) return 0;
    WinVars *vars = self->vars;
    if (!vars || !vars->window) return 0;
    int height;
    SDL_GetWindowSize(vars->window, NULL, &height);
    return height;
}


static void Impl_get_center(Window *self, int *x, int *y) {
    if (!self) return;
    WinVars *vars = self->vars;
    if (!vars || !vars->window) return;
    int w, h;
    SDL_GetWindowSize(vars->window, &w, &h);
    *x = w / 2;
    *y = h / 2;
}


static void Impl_set_position(Window *self, int x, int y) {
    if (!self) return;
    WinVars *vars = self->vars;
    if (!vars || !vars->window) return;

    // Обрабатываем позицию окна (координаты -1 означают выравнивание по центру экрана):
    int pos_x = x, pos_y = y;
    if (x == -1 || y == -1) {
        int ww, wh, dw, dh;
        self->get_size(self, &ww, &wh);
        self->get_display_size(self, self->get_window_display_id(self), &dw, &dh);
        if (x == -1) pos_x = (dw - ww) / 2;
        if (y == -1) pos_y = (dh - wh) / 2;
    }

    SDL_SetWindowPosition(vars->window, pos_x, pos_y);
}


static void Impl_get_position(Window *self, int *x, int *y) {
    if (!self) return;
    WinVars *vars = self->vars;
    if (!vars || !vars->window) return;
    SDL_GetWindowPosition(vars->window, x, y);
}


static void Impl_set_vsync(Window *self, bool vsync) {
    if (!self || !self->config) return;
    SDL_GL_SetSwapInterval(vsync);
    self->config->vsync = vsync;
}


static bool Impl_get_vsync(Window *self) {
    if (!self || !self->config) return false;
    return self->config->vsync;
}


static void Impl_set_fps(Window *self, int fps) {
    if (!self || !self->config) return;
    self->config->fps = fps;
}


static int Impl_get_target_fps(Window *self) {
    if (!self || !self->config) return 0;
    return self->config->fps;
}


static void Impl_set_visible(Window *self, bool visible) {
    if (!self || !self->config) return;
    WinVars *vars = self->vars;
    if (!vars || !vars->window) return;
    if (visible) { SDL_ShowWindow(vars->window); }
    else { SDL_HideWindow(vars->window); }
    self->config->visible = visible;
}


static bool Impl_get_visible(Window *self) {
    if (!self || !self->config) return false;
    return self->config->visible;
}


static void Impl_set_titlebar(Window *self, bool titlebar) {
    if (!self || !self->config) return;
    WinVars *vars = self->vars;
    if (!vars || !vars->window) return;
    SDL_SetWindowBordered(vars->window, titlebar);
    self->config->titlebar = titlebar;
}


static bool Impl_get_titlebar(Window *self) {
    if (!self || !self->config) return false;
    return self->config->titlebar;
}


static void Impl_set_resizable(Window *self, bool resizable) {
    if (!self || !self->config) return;
    WinVars *vars = self->vars;
    if (!vars || !vars->window) return;
    SDL_SetWindowResizable(vars->window, resizable);
    self->config->resizable = resizable;
}


static bool Impl_get_resizable(Window *self) {
    if (!self || !self->config) return false;
    return self->config->resizable;
}


static void Impl_set_fullscreen(Window *self, bool fullscreen) {
    if (!self || !self->config) return;
    WinVars *vars = self->vars;
    if (!vars || !vars->window) return;
    SDL_SetWindowFullscreen(vars->window, fullscreen ? SDL_WINDOW_FULLSCREEN : 0);
    self->config->fullscreen = fullscreen;
}


static bool Impl_get_fullscreen(Window *self) {
    if (!self || !self->config) return false;
    return self->config->fullscreen;
}


static void Impl_set_min_size(Window *self, int width, int height) {
    if (!self) return;
    WinVars *vars = self->vars;
    if (!vars || !vars->window) return;
    SDL_SetWindowMinimumSize(vars->window, width, height);
}


static void Impl_get_min_size(Window *self, int *width, int *height) {
    if (!self) return;
    WinVars *vars = self->vars;
    if (!vars || !vars->window) return;
    SDL_GetWindowMinimumSize(vars->window, width, height);
}


static void Impl_set_max_size(Window *self, int width, int height) {
    if (!self) return;
    WinVars *vars = self->vars;
    if (!vars || !vars->window) return;
    SDL_SetWindowMaximumSize(vars->window, width, height);
}


static void Impl_get_max_size(Window *self, int *width, int *height) {
    if (!self) return;
    WinVars *vars = self->vars;
    if (!vars || !vars->window) return;
    SDL_GetWindowMaximumSize(vars->window, width, height);
}


static void Impl_set_always_top(Window *self, bool on_top) {
    if (!self || !self->config) return;
    WinVars *vars = self->vars;
    if (!vars || !vars->window) return;
    SDL_SetWindowAlwaysOnTop(vars->window, on_top);
    self->config->always_top = on_top;
}


static bool Impl_get_always_top(Window *self) {
    if (!self || !self->config) return false;
    return self->config->always_top;
}


static bool Impl_get_is_focused(Window *self) {
    if (!self) return false;
    WinVars *vars = self->vars;
    if (!vars) return false;
    return vars->focused;
}


static bool Impl_get_is_defocused(Window *self) {
    if (!self) return false;
    WinVars *vars = self->vars;
    if (!vars) return false;
    return vars->defocused;
}


static uint32_t Impl_get_window_display_id(Window *self) {
    if (!self) return 0;
    WinVars *vars = self->vars;
    if (!vars || !vars->window) return 0;
    return (uint32_t)SDL_GetDisplayForWindow(vars->window);
}


static bool Impl_get_display_size(Window *self, uint32_t id, int *width, int *height) {
    if (!self) return false;
    SDL_Rect rect = { 0, 0, 0, 0 };
    if (!SDL_GetDisplayUsableBounds((SDL_DisplayID)id, &rect)) return false;
    *width = rect.w;
    *height = rect.h;
    return true;
}


static void Impl_maximize(Window *self) {
    if (!self) return;
    WinVars *vars = self->vars;
    if (!vars || !vars->window) return;
    SDL_MaximizeWindow(vars->window);
}


static void Impl_minimize(Window *self) {
    if (!self) return;
    WinVars *vars = self->vars;
    if (!vars || !vars->window) return;
    SDL_MinimizeWindow(vars->window);
}


static void Impl_restore(Window *self) {
    if (!self) return;
    WinVars *vars = self->vars;
    if (!vars || !vars->window) return;
    SDL_RestoreWindow(vars->window);
}


static void Impl_raise(Window *self) {
    if (!self) return;
    WinVars *vars = self->vars;
    if (!vars || !vars->window) return;
    SDL_RaiseWindow(vars->window);
}


static float Impl_get_current_fps(Window *self) {
    if (!self) return 0.0f;
    WinVars *vars = self->vars;
    if (!vars) return 0.0f;
    return (float)(1.0/vars->dtime);
}


static double Impl_get_dtime(Window *self) {
    if (!self) return 0.0f;
    WinVars *vars = self->vars;
    if (!vars) return 0.0f;
    return vars->dtime;
}


static double Impl_get_time(Window *self) {
    if (!self) return 0.0;
    WinVars *vars = self->vars;
    if (!vars) return 0.0;
    // Получаем время с начала создания окна в секундах:
    return ((double)SDL_GetPerformanceCounter() / (double)vars->perf_freq) - vars->start_time;
}


static void Impl_clear(Window *self, float r, float g, float b) {
    if (!self) return;
    glClearDepth(1.0f);
    glClearColor(r, g, b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}


static void Impl_display(Window *self) {
    if (!self) return;
    WinVars *vars = self->vars;
    if (!vars || !vars->window) return;
    SDL_GL_SwapWindow(vars->window);
}


// Реализация функций ввода:


static Input_Scancode Convert_scancode(SDL_Scancode scancode) {
    switch (scancode) {
        case SDL_SCANCODE_0: return K_0;
        case SDL_SCANCODE_1: return K_1;
        case SDL_SCANCODE_2: return K_2;
        case SDL_SCANCODE_3: return K_3;
        case SDL_SCANCODE_4: return K_4;
        case SDL_SCANCODE_5: return K_5;
        case SDL_SCANCODE_6: return K_6;
        case SDL_SCANCODE_7: return K_7;
        case SDL_SCANCODE_8: return K_8;
        case SDL_SCANCODE_9: return K_9;
        case SDL_SCANCODE_A: return K_a;
        case SDL_SCANCODE_B: return K_b;
        case SDL_SCANCODE_C: return K_c;
        case SDL_SCANCODE_D: return K_d;
        case SDL_SCANCODE_E: return K_e;
        case SDL_SCANCODE_F: return K_f;
        case SDL_SCANCODE_G: return K_g;
        case SDL_SCANCODE_H: return K_h;
        case SDL_SCANCODE_I: return K_i;
        case SDL_SCANCODE_J: return K_j;
        case SDL_SCANCODE_K: return K_k;
        case SDL_SCANCODE_L: return K_l;
        case SDL_SCANCODE_M: return K_m;
        case SDL_SCANCODE_N: return K_n;
        case SDL_SCANCODE_O: return K_o;
        case SDL_SCANCODE_P: return K_p;
        case SDL_SCANCODE_Q: return K_q;
        case SDL_SCANCODE_R: return K_r;
        case SDL_SCANCODE_S: return K_s;
        case SDL_SCANCODE_T: return K_t;
        case SDL_SCANCODE_U: return K_u;
        case SDL_SCANCODE_V: return K_v;
        case SDL_SCANCODE_W: return K_w;
        case SDL_SCANCODE_X: return K_x;
        case SDL_SCANCODE_Y: return K_y;
        case SDL_SCANCODE_Z: return K_z;

        case SDL_SCANCODE_KP_0: return K_KP_0;
        case SDL_SCANCODE_KP_1: return K_KP_1;
        case SDL_SCANCODE_KP_2: return K_KP_2;
        case SDL_SCANCODE_KP_3: return K_KP_3;
        case SDL_SCANCODE_KP_4: return K_KP_4;
        case SDL_SCANCODE_KP_5: return K_KP_5;
        case SDL_SCANCODE_KP_6: return K_KP_6;
        case SDL_SCANCODE_KP_7: return K_KP_7;
        case SDL_SCANCODE_KP_8: return K_KP_8;
        case SDL_SCANCODE_KP_9: return K_KP_9;
        case SDL_SCANCODE_KP_EXCLAM: return K_KP_EXCLAIM;
        case SDL_SCANCODE_KP_HASH: return K_KP_HASH;
        case SDL_SCANCODE_KP_AMPERSAND: return K_KP_AMPERSAND;
        case SDL_SCANCODE_KP_LEFTPAREN: return K_KP_LEFTPAREN;
        case SDL_SCANCODE_KP_RIGHTPAREN: return K_KP_RIGHTPAREN;
        case SDL_SCANCODE_KP_PERIOD: return K_KP_PERIOD;
        case SDL_SCANCODE_KP_DIVIDE: return K_KP_DIVIDE;
        case SDL_SCANCODE_KP_MULTIPLY: return K_KP_MULTIPLY;
        case SDL_SCANCODE_KP_MINUS: return K_KP_MINUS;
        case SDL_SCANCODE_KP_PLUS: return K_KP_PLUS;
        case SDL_SCANCODE_KP_ENTER: return K_KP_ENTER;
        case SDL_SCANCODE_KP_EQUALS: return K_KP_EQUALS;
        case SDL_SCANCODE_KP_COLON: return K_KP_COLON;
        case SDL_SCANCODE_KP_LESS: return K_KP_LESS;
        case SDL_SCANCODE_KP_GREATER: return K_KP_GREATER;
        case SDL_SCANCODE_KP_AT: return K_KP_AT;

        case SDL_SCANCODE_F1: return K_F1;
        case SDL_SCANCODE_F2: return K_F2;
        case SDL_SCANCODE_F3: return K_F3;
        case SDL_SCANCODE_F4: return K_F4;
        case SDL_SCANCODE_F5: return K_F5;
        case SDL_SCANCODE_F6: return K_F6;
        case SDL_SCANCODE_F7: return K_F7;
        case SDL_SCANCODE_F8: return K_F8;
        case SDL_SCANCODE_F9: return K_F9;
        case SDL_SCANCODE_F10: return K_F10;
        case SDL_SCANCODE_F11: return K_F11;
        case SDL_SCANCODE_F12: return K_F12;
        case SDL_SCANCODE_F13: return K_F13;
        case SDL_SCANCODE_F14: return K_F14;
        case SDL_SCANCODE_F15: return K_F15;

        case SDL_SCANCODE_BACKSPACE: return K_BACKSPACE;
        case SDL_SCANCODE_TAB: return K_TAB;
        case SDL_SCANCODE_CLEAR: return K_CLEAR;
        case SDL_SCANCODE_RETURN: return K_RETURN;
        case SDL_SCANCODE_PAUSE: return K_PAUSE;
        case SDL_SCANCODE_ESCAPE: return K_ESCAPE;
        case SDL_SCANCODE_SPACE: return K_SPACE;
        case SDL_SCANCODE_COMMA: return K_COMMA;
        case SDL_SCANCODE_MINUS: return K_MINUS;
        case SDL_SCANCODE_PERIOD: return K_PERIOD;
        case SDL_SCANCODE_SLASH: return K_SLASH;
        case SDL_SCANCODE_SEMICOLON: return K_SEMICOLON;
        case SDL_SCANCODE_EQUALS: return K_EQUALS;
        case SDL_SCANCODE_LEFTBRACKET: return K_LEFTBRACKET;
        case SDL_SCANCODE_BACKSLASH: return K_BACKSLASH;
        case SDL_SCANCODE_RIGHTBRACKET: return K_RIGHTBRACKET;
        case SDL_SCANCODE_DELETE: return K_DELETE;
        case SDL_SCANCODE_UP: return K_UP;
        case SDL_SCANCODE_DOWN: return K_DOWN;
        case SDL_SCANCODE_RIGHT: return K_RIGHT;
        case SDL_SCANCODE_LEFT: return K_LEFT;
        case SDL_SCANCODE_INSERT: return K_INSERT;
        case SDL_SCANCODE_HOME: return K_HOME;
        case SDL_SCANCODE_END: return K_END;
        case SDL_SCANCODE_PAGEUP: return K_PAGEUP;
        case SDL_SCANCODE_PAGEDOWN: return K_PAGEDOWN;
        case SDL_SCANCODE_NUMLOCKCLEAR: return K_NUMLOCK;
        case SDL_SCANCODE_CAPSLOCK: return K_CAPSLOCK;
        case SDL_SCANCODE_SCROLLLOCK: return K_SCROLLLOCK;
        case SDL_SCANCODE_RSHIFT: return K_RSHIFT;
        case SDL_SCANCODE_LSHIFT: return K_LSHIFT;
        case SDL_SCANCODE_RCTRL: return K_RCTRL;
        case SDL_SCANCODE_LCTRL: return K_LCTRL;
        case SDL_SCANCODE_RALT: return K_RALT;
        case SDL_SCANCODE_LALT: return K_LALT;
        case SDL_SCANCODE_RGUI: return K_RGUI;
        case SDL_SCANCODE_LGUI: return K_LGUI;
        case SDL_SCANCODE_MODE: return K_MODE;
        case SDL_SCANCODE_HELP: return K_HELP;
        case SDL_SCANCODE_PRINTSCREEN: return K_PRINTSCREEN;
        case SDL_SCANCODE_SYSREQ: return K_SYSREQ;
        case SDL_SCANCODE_MENU: return K_MENU;
        case SDL_SCANCODE_POWER: return K_POWER;

        // Other keys...

        case SDL_SCANCODE_UNKNOWN:
        default: return K_UNKNOWN;
    }
}


static void Impl_set_mouse_pos(Window *self, int x, int y) {
    if (!self || !self->input || !self->input->keyboard) return;
    WinVars *vars = self->vars;
    if (!vars || !vars->window) return;
    SDL_WarpMouseInWindow(vars->window, x, y);
}


static void Impl_set_mouse_visible(Window *self, bool visible) {
    if (!self || !self->input || !self->input->keyboard) return;
    visible ? SDL_ShowCursor() : SDL_HideCursor();
    self->input->mouse->visible = visible;
}
