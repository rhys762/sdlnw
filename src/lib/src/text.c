#include "SDLNW.h"
#include "internal_helpers.h"
#include <assert.h>

struct text_data {
    SDLNW_TextWidgetOptions options;

    SDLNW_TextControllerChangeListener* tccl;

    bool selected;
    size_t cursor;

    __sdlnw_RenderedText text;
    
    size_t selection_start, selection_end;
    bool is_dragging;
};

static void text_draw(SDLNW_Widget* w, SDL_Renderer* renderer) {
    struct text_data* data = w->data;

    __sdlnw_RenderedText_render_text(&data->text, renderer);
    
    if (data->options.selectable && (data->selection_start || data->selection_end)) {
        __sdlnw_RenderedText_render_highlight(&data->text, renderer, data->selection_start, data->selection_end, &data->options.highlight);
    }

    // render cursor, ocillate every half second
    if (data->selected && data->options.editable) {
        Uint32 tick = SDL_GetTicks() / 500;
        if (tick % 2) {
            __sdlnw_RenderedText_render_cursor(&data->text, renderer, data->cursor, &(SDLNW_Colour) {0x00, 0x00, 0x00, 0xFF});
        }
    }
}

static bool is_whitespace(char c) {
    return c == ' ' || c == '\t';
}

static void text_click(SDLNW_Widget* w, SDLNW_Event_Click* event, bool* allow_passthrough) {
    struct text_data* data = w->data;

    if (is_point_within_rect(event->x, event->y, &w->size)) {
        *allow_passthrough = false;
        data->selected = true;

        if (event->clicks == 1) {
            // set cursor
            data->cursor = __sdlnw_RenderedText_convert_point_to_char_index(&data->text, event->x, event->y);\
            if (data->cursor == (size_t)-1) {
                // not found, just put at 0
                data->cursor = 0;
            }

            data->selection_start = 0;
            data->selection_end = 0;
        }
        else if (event->clicks == 2) {
            // select the word the user clicks on and set cursor to end
            // if user clicks whitespace select the entire whitespace
            // set cursor
            size_t idx = __sdlnw_RenderedText_convert_point_to_char_index(&data->text, event->x, event->y);
            if (idx == (size_t)-1) {
                // not found, just put at 0
                data->cursor = 0;
                data->selection_start = 0;
                data->selection_end = 0;
            }
            else {
                const char* str = SDLNW_TextController_get_value(data->options.text_controller);
                bool looking_for_whitespace = is_whitespace(str[idx]);
                size_t len = strlen(str);

                data->selection_start = idx;
                data->selection_end = idx;

                while (data->selection_start > 0 && str[data->selection_start - 1] != '\n' && is_whitespace(str[data->selection_start - 1]) == looking_for_whitespace) {
                    data->selection_start--;
                }

                while (data->selection_end < len && str[data->selection_end] != '\n' && is_whitespace(str[data->selection_end]) == looking_for_whitespace) {
                    data->selection_end++;
                }

                data->cursor = data->selection_end;
            }
        }

        
        
    } else {
        data->selected = false;
        return;
    }
}

static void text_drag(SDLNW_Widget* widget, SDLNW_Event_Drag* event, bool* allow_passthrough) {
    (void)allow_passthrough; // TODO, unsure how to handle
    struct text_data* data = widget->data;

    if (!data->is_dragging) {
        if (is_point_within_rect(event->mouse_x, event->mouse_y, &widget->size)) {
            size_t idx = __sdlnw_RenderedText_convert_point_to_char_index(&data->text, event->origin_x, event->origin_y);
            if (idx != (size_t)-1) {
                data->selection_start = idx;
                data->is_dragging = true;
                data->selection_end = data->selection_start;
            }
        }
    } else {
        size_t idx = __sdlnw_RenderedText_convert_point_to_char_index(&data->text, event->mouse_x, event->mouse_y);
        if (idx != (size_t)-1) {
            data->selection_end = idx;
        }

        data->cursor = data->selection_end;

        if (!event->still_down) {
            data->is_dragging = false;
        }
    }
}

static size_t min(size_t a, size_t b) {
    if (a < b) {
        return a;
    }

    return b;
}

static size_t max(size_t a, size_t b) {
    if (a < b) {
        return b;
    }

    return a;
}

static void clear_selection(struct text_data* data) {
    size_t low = min(data->selection_start, data->selection_end);
    size_t high = max(data->selection_start, data->selection_end);

    size_t to_clear = high - low;

    if (!to_clear) {
        return;
    }

    for (size_t i = 0; i < to_clear; i++) {
        SDLNW_TextController_remove(data->options.text_controller, low);
    }

    data->selection_start = 0;
    data->selection_end = 0;
    data->is_dragging = 0;
    data->cursor = low;
}

static void copy_selection(struct text_data* data) {
    size_t low = min(data->selection_start, data->selection_end);
    size_t high = max(data->selection_start, data->selection_end);

    size_t len = 1 + high - low;

    const char* str = SDLNW_TextController_get_value(data->options.text_controller);
    char* buffer = __sdlnw_malloc(sizeof(char) * len);

    for (size_t i = low; i < high; i++) {
        buffer[i-low] = str[i];
    }
    buffer[len - 1] = '\0';

    SDL_SetClipboardText(buffer);

    __sdlnw_free(buffer);
}

static void paste(struct text_data* data, const char* text) {
    int len = strlen(text);

    for (int i = 0; i < len; i++) {
        SDLNW_TextController_insert(data->options.text_controller, text[i], data->cursor);
        data->cursor++;
    }
}

static void text_on_key_up(SDLNW_Widget* w, SDLNW_Event_KeyUp* event, bool* allow_passthrough) {
    (void)w;
    (void)event;
    (void)allow_passthrough;

    struct text_data* data = w->data;

    if (!data->selected) {
        // not selected, ignore
        return;
    }
    
    SDL_KeyCode c = event->key;
    const char* str = SDLNW_TextController_get_value(data->options.text_controller);
    SDL_Keymod mod_state = SDL_GetModState();

    // TODO newline? up and down arrow keys for multi-line?
    if (c == SDLK_BACKSPACE) {
        if (data->options.editable) {
            if (data->selection_start || data->selection_end) {
                clear_selection(data);
            }
            else if (data->cursor > 0) {
                data->cursor--;
                SDLNW_TextController_remove(data->options.text_controller, data->cursor);
            }
        }
    } else if (c == SDLK_DELETE) {
        if (data->options.editable) {
            if (data->selection_start || data->selection_end) {
                clear_selection(data);
            } else if (data->cursor < strlen(str)) {
                SDLNW_TextController_remove(data->options.text_controller, data->cursor);
            }
        }
    } else if (c == SDLK_LEFT) {
        if (data->cursor > 0) {
            data->cursor--;
        }
    } else if (c == SDLK_RIGHT) {
        if (data->cursor < strlen(str)) {
            data->cursor++;
        }
    } else if (c == SDLK_HOME) { // TODO multiline
        data->cursor = 0;
    } else if (c == SDLK_END) { // TODO multiline
        size_t len = strlen(str);
        if (len) {
            data->cursor = len;
        }
    } else if (c == SDLK_c && (mod_state & KMOD_CTRL)) {
        if (data->selection_start || data->selection_end) {
            copy_selection(data);
        }
    } else if (c == SDLK_v && (mod_state & KMOD_CTRL)) {
        if (data->options.editable) {
            char* clipboard = SDL_GetClipboardText();
            if (clipboard != NULL) {
                if (data->selection_start || data->selection_end) {
                    clear_selection(data);
                    paste(data, clipboard);
                } else {
                    paste(data, clipboard);
                }
            }
            SDL_free(clipboard);
        }
    } else if (c == SDLK_RETURN) {
        if (data->options.editable) {
            SDLNW_TextController_insert(data->options.text_controller, '\n', data->cursor);
            data->cursor += 1;
        }
    }
}

static void text_on_text_input(SDLNW_Widget* widget, SDLNW_Event_TextInput* event, bool* allow_passthrough) {
    (void)allow_passthrough; // ignore

    struct text_data* data = widget->data;

    if (!data->selected) {
        // not selected, ignore
        return;
    }

    if (!data->options.editable) {
        return;
    }

    if (data->selection_start || data->selection_end) {
        clear_selection(data);
    }

    size_t len = strlen(event->text);
    for (size_t i = 0; i < len; i++) {
        SDLNW_TextController_insert(data->options.text_controller, event->text[i], data->cursor);
        data->cursor++;
    }
}

static SDLNW_SizeResponse text_get_requested_size(SDLNW_Widget* widget, SDLNW_SizeRequest request) {
    struct text_data* data = widget->data;

    SDLNW_SizeResponse req = (SDLNW_SizeResponse){0};

    if (request.total_pixels_avaliable_width) {
        // create a rendered text to do a live check, don't want our actual one to be modified.
        __sdlnw_RenderedText rt;
        __sdlnw_RenderedText_init(&rt, data->options.font);
        
        SDL_Rect dim = {
            .w = request.total_pixels_avaliable_width
        };

        __sdlnw_RenderedText_set_text(&rt, SDLNW_TextController_get_value(data->options.text_controller), &dim);

        if (rt.chars_len > 0) {
            __sdlnw_RenderedChar* c = &rt.chars[rt.chars_len - 1];
            req.height = (SDLNW_DimensionSizeRequest) {
                .pixels = c->rendered_to.y + c->rendered_to.h
            };
        }
        

        __sdlnw_RenderedText_destroy(&rt);
    }

    return req;
}

static void text_size(SDLNW_Widget* w, const SDL_Rect* rect) {
    struct text_data* data = w->data;

    w->size = *rect;
    __sdlnw_RenderedText_set_text(&data->text, SDLNW_TextController_get_value(data->options.text_controller), &w->size);
}

static void text_destroy(SDLNW_Widget* w) {
    struct text_data* data = w->data;

    __sdlnw_RenderedText_destroy(&data->text);
    
    // if we dont remove listener, the text controller probably got destroyed which
    // destroys the text listener.
    if (SDLNW_TextController_remove_change_listener(data->options.text_controller, data->tccl)) {
        SDLNW_TextControllerChangeListener_Destroy(data->tccl);
    }

    __sdlnw_free(w->data);
    w->data = NULL;
}

static void listener(void* d, char* text) {
    (void)text;

    SDLNW_Widget* widget = d;
    struct text_data* data = widget->data;

    __sdlnw_RenderedText_set_text(&data->text, text, &widget->size);
}

SDLNW_Widget* SDLNW_CreateTextWidget(SDLNW_TextWidgetOptions options) {
    assert(options.text_controller != NULL);

    if (options.editable) {
        assert(options.selectable != false);
    }

    SDLNW_Widget* widget = create_default_widget();

    widget->vtable.destroy = text_destroy;
    widget->vtable.draw = text_draw;
    widget->vtable.click = text_click;
    widget->vtable.drag = text_drag;
    widget->vtable.on_key_up = text_on_key_up;
    widget->vtable.on_text_input = text_on_text_input;
    widget->vtable.size = text_size;
    widget->vtable.get_requested_size = text_get_requested_size;

    widget->data = __sdlnw_malloc(sizeof(struct text_data));
    struct text_data* p = widget->data;
    *p = (struct text_data){
        .options = options
    };

    __sdlnw_RenderedText_init(&p->text, options.font);

    p->tccl = SDLNW_CreateTextControllerChangeListener(widget, listener, NULL);

    SDLNW_TextController_add_change_listener(options.text_controller, p->tccl);

    __sdlnw_RenderedText_set_text(&p->text, SDLNW_TextController_get_value(options.text_controller), &widget->size);

    return widget;
}