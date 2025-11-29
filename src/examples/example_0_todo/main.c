#include <assert.h>
#include <libgen.h>

#include "SDLNW.h"
#include "SDLNWPrefab.h"
#include "SDL_timer.h"

#define RED (SDL_Colour) {.r = 0xFF, .a = 0xFF}
#define WHITE (SDL_Colour) {.r = 0xFF, .g = 0xFF, .b = 0xFF, .a = 0xFF}
#define BLACK (SDL_Colour) {.a = 0xFF}
#define ORANGE (SDL_Colour) {.r = 0xFF, .g=0xA5, .a = 0xFF}
#define GOLD (SDL_Colour) {.r = 0xFF, .g=0xD7, .a = 0xFF}

SDLNW_Font* FONT = NULL;

typedef struct {
    bool is_done;
    char* text;
} TodoItem;

typedef struct {
    TodoItem* items;
    size_t items_len;
    size_t items_cap;

    SDLNW_TextController add_item_text_controller;
    SDLNW_Widget* task_display;
    SDLNW_Widget* root_widget;
} AppState;


TodoItem TodoItem_create(bool is_done, char* text) {
    TodoItem item = {
        .is_done = is_done
    };

    size_t len = strlen(text) + 1;
    item.text = malloc(sizeof(char) * len);
    strcpy(item.text, text);

    return item;
}

void TodoItem_destroy(TodoItem* item) {
    free(item->text);
    *item = (TodoItem){0};
}

AppState AppState_create(void) {
    AppState appstate = {0};

    appstate.items_cap = 10;
    appstate.items = malloc(appstate.items_cap * sizeof(TodoItem));

    SDLNW_InitTextController(&appstate.add_item_text_controller);

    return appstate;
}

void AppState_add_item(AppState* appstate, bool is_done, char* text) {
    TodoItem item = TodoItem_create(is_done, text);

    if (appstate->items_len >= appstate->items_cap) {
        appstate->items_cap *= 2;
        appstate->items = realloc(appstate->items, appstate->items_cap * sizeof(TodoItem));
        assert(appstate->items != NULL);
    }

    appstate->items[appstate->items_len] = item;
    appstate->items_len++;
}

void AppState_remove(AppState* appstate, size_t idx) {
    if (idx >= appstate->items_len) {
        return;
    }

    TodoItem_destroy(&appstate->items[idx]);

    appstate->items_len--;
    for (size_t i = idx; i < appstate->items_len; i++) {
        appstate->items[i] = appstate->items[i + 1];
    }
}

void AppState_destroy(AppState* appstate) {
    free(appstate->items);
    SDLNW_DestroyTextController(&appstate->add_item_text_controller);
    *appstate = (AppState){0};
}

static void on_title_destroy(void* tc) {
    SDLNW_DestroyTextController((SDLNW_TextController*)tc);
    free(tc);
}

SDLNW_Widget* create_title(void) {
    SDLNW_TextController* tc = malloc(sizeof(SDLNW_TextController));
    SDLNW_InitTextController(tc);
    SDLNW_SetTextControllerValue(tc, "Todo List");

    SDLNW_TextWidgetOptions options = {
        .text_controller = tc,
        .font = FONT,
        .fg = BLACK
    };

    SDLNW_Widget* title = SDLNW_CreateTextWidget(options);
    SDLNW_AddOnWidgetDestroyCb(title, tc, on_title_destroy);

    return title;
}

void create_task_button_click(void* data) {
    AppState* app_state = (AppState*)data;

    char* text = SDLNW_GetTextControllerValue(&app_state->add_item_text_controller);

    if (!strlen(text)) {
        return;
    }

    AppState_add_item(app_state, false, text);
    SDLNW_SetTextControllerValue(&app_state->add_item_text_controller, "");

    SDLNW_RecomposeWidget(app_state->task_display);
}

SDLNW_Widget* create_task_entry(AppState* app_state) {
    SDLNW_TextWidgetOptions options = {
        .text_controller = &app_state->add_item_text_controller,
        .font = FONT,
        .fg = BLACK,
        .editable = true,
        .highlight = RED,
        .selectable = true,
    };

    SDLNW_Widget* entry = SDLNW_CreateTextWidget(options);
    SDLNW_SetWidgetBorder(entry, SDLNW_CreateSolidBorder(2, (SDL_Colour) {
        .a = 0xFF
    }));
    SDLNW_SetWidgetCornerRadius(entry, (SDLNW_CornerRadius) {
        .top_left = 2,
        .top_right = 2,
        .bottom_left = 2,
        .bottom_right = 2
    });
    SDLNW_SetWidgetPadding(entry, (SDLNW_Insets) {
        .top = 3,
        .right = 3,
        .bottom = 3,
        .left = 3
    });

    SDLNW_Widget* sized_entry = SDLNW_CreateSizedBoxWidget(entry, (SDLNW_SizedBoxWidgetOptions) {
        .width_shares = 1
    });

    SDLNW_Widget* button = SDLNWPrefab_CreateButton((SDLNWPrefab_ButtonOptions) {
        .text = "Add Task",
        .cb_data = app_state,
        .cb = create_task_button_click,
        .font = FONT
    });

    SDLNW_Widget* widgets[] = {
        sized_entry,
        button,
        NULL
    };

    return SDLNW_CreateRowWidget(widgets);
}

struct mark_complete_info {
    AppState* app_state;
    size_t idx;
};

void mark_complete(void* data) {
    struct mark_complete_info* info = data;
    AppState_remove(info->app_state, info->idx);
    SDLNW_RecomposeWidget(info->app_state->task_display);
}

SDLNW_Widget* task_display_compose(SDLNW_Widget* parent, void* data) {
    (void)parent;
    AppState* app_state = data;

    SDLNW_Widget** rows = malloc(sizeof(SDLNW_Widget*) * app_state->items_len + 1);

    for (size_t i = 0; i < app_state->items_len; i++) {
        struct mark_complete_info* info = malloc(sizeof(struct mark_complete_info));
        *info = (struct mark_complete_info) {
            .app_state = app_state,
            .idx = i
        };

        SDLNW_Widget* button = SDLNWPrefab_CreateButton((SDLNWPrefab_ButtonOptions) {
            .text = "Complete",
            .cb = mark_complete,
            .cb_data = info,
            .font = FONT
        });
        SDLNW_AddOnWidgetDestroyCb(button, info, free);

        SDLNW_Widget* row[] = {
            SDLNW_CreateSizedBoxWidget(SDLNWPrefab_CreateLabelWidget(app_state->items[i].text, FONT, BLACK), (SDLNW_SizedBoxWidgetOptions) {
                .width_shares = 1
            }),
            button,
            NULL
        };

        rows[i] = SDLNW_CreateRowWidget(row);
    }

    rows[app_state->items_len] = NULL; // null terminated

    SDLNW_Widget* column = SDLNW_CreateColumnWidget(rows);
    free(rows);

    SDLNW_Widget* scroll = SDLNW_CreateScrollWidget(column);

    return scroll;
}

SDLNW_Widget* create_task_display(AppState* app_state) {
    SDLNW_Widget* task_display = SDLNW_CreateCompositeWidget(app_state, task_display_compose);

    app_state->task_display = task_display;

    return task_display;

}

SDLNW_Widget* create_root_widget(AppState* app_state) {
    SDLNW_Widget* widgets[] = {
        create_title(),
        create_task_entry(app_state),
        create_task_display(app_state),
        NULL
    };

    SDLNW_Widget* column = SDLNW_CreateColumnWidget(widgets);

    SDLNW_SetWidgetBackground(column, SDLNW_CreateSolidBackground((SDL_Color){
        .r = 0xFF,
        .g = 0xFF,
        .b = 0xFF,
        .a = 0xFF
    }));

    return column;
}

int main(int argc, char** argv) {
    // no args, use relative path to find the ttf file.
    assert(argc == 1);
    const char* dir = dirname(argv[0]);
    char* fontpath = NULL;
    SDL_asprintf(&fontpath, "%s/../jbm/fonts/ttf/JetBrainsMonoNL-Regular.ttf", dir);

    // need to init ttf for font rendering
    SDL_Init(SDL_INIT_EVERYTHING);
    TTF_Init();

    FONT = SDLNW_CreateFont(fontpath, 20);
    assert(FONT != NULL);

    SDLNW_WidgetWindowSet* widget_window_set = SDLNW_CreateWidgetWindowSet();

    AppState app_state = AppState_create();

    // create the composite widget and put it in a window
    SDLNW_Widget* widget = create_root_widget(&app_state);
    app_state.root_widget = widget;
    const SDLNW_WidgetWindowOptions options = (SDLNW_WidgetWindowOptions) {
        .title = "Todo App",
        .sdl_window_flags = SDL_WINDOW_RESIZABLE
    };
    SDLNW_WidgetWindowSet_CreateWidgetWindow(widget_window_set, widget, options);

    size_t frames = 0;
    Uint32 last_t = SDL_GetTicks(), acc_t = 0;

    while (SDLNW_CreateWidgetWindowSet_get_number_of_windows(widget_window_set)) {
        SDLNW_CreateWidgetWindowSet_step(widget_window_set);

        Uint32 t = SDL_GetTicks();
        Uint32 delta_t = t - last_t;
        last_t = t;
        frames++;

        acc_t += delta_t;

        if (acc_t >= 1000) {
            printf("handled %lu frames in %u milli's\n", frames, acc_t);
            acc_t = 0;
            frames = 0;
        }
    }

    AppState_destroy(&app_state);

    SDLNW_DestroyWidgetWindowSet(widget_window_set);

    free(fontpath);
    fontpath = NULL;

    SDLNW_DestroyFont(FONT);

    TTF_Quit();
    SDL_Quit();

    SDLNW_ReportLeaks();

    return 0;
}
