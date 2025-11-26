#include "SDLNW.h"
#include "SDLNWPrefab.h"
#include "SDLNWInternal.h"

struct button_data {
    SDLNWPrefab_ButtonOptions options;
    bool hovered;
    SDLNW_Widget* self;
};

static void button_on_click(void* data, int x, int y, bool* allow_passthrough) {
    (void)x;
    (void)y;
    struct button_data* bd = data;

    *allow_passthrough = false;

    bd->options.cb(bd->options.cb_data);
}

static void button_on_mouse_hover_on(void* data, bool* allow_passthrough) {
    (void)allow_passthrough;
    struct button_data* bd = data;

    bd->hovered = true;
    SDLNW_RecomposeWidget(bd->self);
}

static void button_on_mouse_hover_off(void* data, bool* allow_passthrough) {
    (void)allow_passthrough;
    struct button_data* bd = data;

    bd->hovered = false;
    SDLNW_RecomposeWidget(bd->self);
}

static SDLNW_Widget* button_compose(SDLNW_Widget* parent, void* data) {
    (void)parent;
    struct button_data* bd = data;

    SDL_Colour colour = (bd->hovered) ? bd->options.button_hover_bg : bd->options.button_bg;

    int radius = 7;

    SDLNW_CornerRadius corners = {
        .top_left = radius,
        .top_right = radius,
        .bottom_left = radius,
        .bottom_right = radius
    };

    SDLNW_Widget* label = SDLNWPrefab_CreateLabelWidget(bd->options.text, bd->options.font, bd->options.text_fg);

    SDLNW_SetWidgetPadding(label, (SDLNW_Insets) {
        .top = 5,
        .left = 5,
        .right = 5,
        .bottom = 5
    });

    SDLNW_SetWidgetMargin(label, (SDLNW_Insets) {
        .top = 5,
        .left = 5,
        .right = 5,
        .bottom = 5
    });

    SDLNW_SetWidgetBackground(label, SDLNW_CreateSolidBackground(colour));

    SDLNW_SetWidgetCornerRadius(label, corners);

    SDLNW_GestureDetectorWidgetOptions goptions = {
        .data = data,
        .on_mouse_hover_on = button_on_mouse_hover_on,
        .on_mouse_hover_off = button_on_mouse_hover_off,
        .on_click = button_on_click
    };

    return SDLNW_CreateGestureDetectorWidget(label, goptions);
}

static bool is_zero_colour(const SDL_Colour* c) {
    return !c->r && !c->g && !c->b && !c->a;
}

void destroy_data(void* data) {
    __sdlnw_free(data);
}

SDLNW_Widget* SDLNWPrefab_CreateButton(SDLNWPrefab_ButtonOptions options) {
    struct button_data* d = __sdlnw_malloc(sizeof(struct button_data));

    if (is_zero_colour(&options.button_bg)) {
        options.button_bg = (SDL_Colour) {
            .r = 66,
            .g = 133,
            .b = 244,
            .a = 0xFF
        };
    }

    if (is_zero_colour(&options.button_hover_bg)) {
        options.button_hover_bg = (SDL_Colour) {
            .r = 76,
            .g = 143,
            .b = 254,
            .a = 0xFF
        };
    }

    if (is_zero_colour(&options.text_fg)) {
        options.text_fg = (SDL_Colour) {
            .r = 0xFF,
            .g = 0xFF,
            .b = 0xFF,
            .a = 0xFF
        };
    }

    *d = (struct button_data) {
        .options = options
    };

    SDLNW_Widget* composite = SDLNW_CreateCompositeWidget(d, button_compose);
    d->self = composite;
    SDLNW_AddOnWidgetDestroyCb(composite, d, destroy_data);

    return composite;
}
