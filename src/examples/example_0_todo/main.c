#include <assert.h>
#include <libgen.h>

#include "SDLNW.h"
#include "SDLNWPrefab.h"
#include "SDL_timer.h"
#include "misc.h"
#include "button.h"

static void on_title_destroy(void* tc) {
    SDLNW_TextController_destroy((SDLNW_TextController*)tc);
    free(tc);
}

SDLNW_Widget* create_title(void) {
    SDLNW_TextController* tc = malloc(sizeof(SDLNW_TextController));
    SDLNW_TextController_init(tc);
    SDLNW_TextController_set_value(tc, "Todo List");

    SDLNW_TextWidgetOptions options = {
        .text_controller = tc,
        .font = FONT,
        .fg = BLACK
    };

    SDLNW_Widget* title = SDLNW_CreateTextWidget(options);
    SDLNW_Widget_AddOnDestroy(title, tc, on_title_destroy);

    return title;
}

void create_task_button_click(void* data) {
    AppState* app_state = (AppState*)data;

    char* text = SDLNW_TextController_get_value(&app_state->add_item_text_controller);

    if (!strlen(text)) {
        return;
    }

    AppState_add_item(app_state, false, text);
    SDLNW_TextController_set_value(&app_state->add_item_text_controller, "");

    SDLNW_Widget_Recompose(app_state->task_display);
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
    SDLNW_Widget_add_border(entry, SDLNW_Border_create_solid(2, (SDL_Colour) {
        .a = 0xFF
    }));
    SDLNW_Widget_set_corner_radius(entry, (SDLNW_CornerRadius) {
        .top_left = 2,
        .top_right = 2,
        .bottom_left = 2,
        .bottom_right = 2
    });

    SDLNW_Widget* sized_entry = SDLNW_CreateSizedBoxWidget(entry, (SDLNW_SizedBoxWidget_Options) {
        .width_shares = 1
    });

    SDLNW_Widget* button = create_button("Add Task", app_state, create_task_button_click);

    SDLNW_Widget* widgets[] = {
        sized_entry,
        button,
        NULL
    };

    return SDLNW_CreateRowWidget(widgets);
}

void mark_complete(void* data) {
    TodoItem* item = (TodoItem*)data;
    item->is_done = true;
}

SDLNW_Widget* task_display_compose(SDLNW_Widget* parent, void* data) {
    (void)parent;
    AppState* app_state = data;

    SDLNW_Widget** rows = malloc(sizeof(SDLNW_Widget*) * app_state->items_len + 1);

    for (size_t i = 0; i < app_state->items_len; i++) {
        SDLNW_Widget* row[] = {
            SDLNW_CreateSizedBoxWidget(SDLNWPrefab_CreateLabelWidget(app_state->items[i].text, FONT, BLACK), (SDLNW_SizedBoxWidget_Options) {
                .width_shares = 1
            }),
            create_button("Complete", &app_state->items[i], mark_complete),
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
    // TODO setter
    column->background = SDLNW_CreateSolidBackground((SDL_Color){
        .r = 0xFF,
        .g = 0xFF,
        .b = 0xFF,
        .a = 0xFF
    });

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

    FONT = SDLNW_Font_Create(fontpath, 20);
    assert(FONT != NULL);

    SDLNW_WidgetWindowSet* widget_window_set = SDLNW_CreateWidgetWindowSet();

    AppState app_state = AppState_create();

    // create the composite widget and put it in a window
    SDLNW_Widget* widget = create_root_widget(&app_state);
    app_state.root_widget = widget;
    const SDLNW_BootstrapOptions options = (SDLNW_BootstrapOptions){.sdl_window_flags = SDL_WINDOW_RESIZABLE};
    SDLNW_WidgetWindowSet_CreateWidgetWindow(widget_window_set, widget, options);

    size_t frames = 0;
    Uint32 last_t = SDL_GetTicks(), acc_t;

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

    SDLNW_WidgetWindowSet_destroy(widget_window_set);

    free(fontpath);
    fontpath = NULL;

    SDLNW_Font_Destroy(FONT);

    TTF_Quit();
    SDL_Quit();

    SDLNW_debug_report_leaks();

    return 0;
}
