#include "button.h"
#include "SDLNW.h"
#include "SDLNWPrefab.h"

struct button_data {
    AppState* appstate;
    SDLNW_Widget* parent;
    const char* text;
    bool hovered;

    void* cb_data;
    void (*cb)(void*cb_data);
};

static void button_on_click(void* data, int x, int y, bool* allow_passthrough) {
    (void)x;
    (void)y;
    struct button_data* bd = data;

    *allow_passthrough = false;

    bd->cb(bd->cb_data);
}

static void button_on_mouse_hover_on(void* data, bool* allow_passthrough) {
    (void)allow_passthrough;
    struct button_data* bd = data;

    bd->hovered = true;
    SDLNW_Widget_Recompose(bd->parent);
}

static void button_on_mouse_hover_off(void* data, bool* allow_passthrough) {
    (void)allow_passthrough;
    struct button_data* bd = data;

    bd->hovered = false;
    SDLNW_Widget_Recompose(bd->parent);
}

static SDLNW_Widget* button_compose(SDLNW_Widget* parent, void* data) {
    (void)parent;
    struct button_data* bd = data;

    SDL_Colour colour = (bd->hovered) ? GOLD : ORANGE;

    int radius = 7;

    SDLNW_CornerRadius corners = {
        .top_left = radius,
        .top_right = radius,
        .bottom_left = radius,
        .bottom_right = radius
    };

    SDLNW_Widget* label = SDLNWPrefab_CreateLabelWidget(bd->text, FONT, BLACK);

    SDLNW_Widget_set_padding(label, (SDLNW_Insets) {
        .top = 15,
        .left = 15,
        .right = 15,
        .bottom = 15
    });

    SDLNW_GestureDetectorWidget_Options goptions = {
        .data = data,
        .on_mouse_hover_on = button_on_mouse_hover_on,
        .on_mouse_hover_off = button_on_mouse_hover_off,
        .on_click = button_on_click
    };

    SDLNW_Widget_add_border(label, SDLNW_Border_create_solid(2, (SDL_Color) {
        .a = 0xFF
    }));

    label->background = SDLNW_CreateSolidBackground(colour);

    SDLNW_Widget_set_corner_radius(label, corners);

    return SDLNW_CreateGestureDetectorWidget(label, goptions);
}

SDLNW_Widget* create_button(const char* text, void* cb_data, void (*cb)(void*cb_data)) {
    struct button_data* d = malloc(sizeof(struct button_data));
    *d = (struct button_data) {
        .text = text,
        .cb_data = cb_data,
        .cb = cb
    };

    SDLNW_Widget* composite = SDLNW_CreateCompositeWidget(d, button_compose);

    d->parent = composite;
    SDLNW_Widget_AddOnDestroy(composite, d, free);

    return composite;
}
