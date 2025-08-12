#include "../include/internal_helpers.h"
#include "../include/SDLNW.h"

static void base_draw(SDLNW_Widget* w, SDL_Renderer* renderer) {
    (void)w; // unused
    (void)renderer; // unused
}

static void base_size(SDLNW_Widget* w, const SDL_Rect* rect) {
    w->size = *rect;
}

static void base_click(SDLNW_Widget* w, SDLNW_Event_Click* event, bool* allow_passthrough) {
    (void)w; // unused
    (void)event; // unused
    (void)allow_passthrough; // unused
}

static void base_destroy(SDLNW_Widget* w) {
    __sdlnw_free(w->data);
    w->data = NULL;
}

static SDL_SystemCursor base_appropriate_cursor(SDLNW_Widget* w, int x, int y) {
    (void)w; // unused
    (void)x; // unused
    (void)y; // unused
    return SDL_SYSTEM_CURSOR_ARROW;
}

static SDLNW_SizeResponse base_get_requested_size(SDLNW_Widget* w, SDLNW_SizeRequest request) {
    (void)w; // unused
    (void)request; // unused

    SDLNW_SizeResponse req = (SDLNW_SizeResponse){0};

    return req;
}

static void base_trickle_down_event(SDLNW_Widget* widget, enum SDLNW_EventType type, void* event_meta, bool* allow_passthrough) {
    (void)widget; // unused
    (void)type; // unused
    (void)event_meta; // unused
    (void)allow_passthrough; // unused
}

static void base_mouse_scroll(SDLNW_Widget* widget, SDLNW_Event_MouseWheel* event, bool* allow_passthrough) {   
    (void)widget; // unused
    (void)event; // unused
    (void)allow_passthrough; // unused
}

static void base_drag(SDLNW_Widget* widget, SDLNW_Event_Drag* event, bool* allow_passthrough) {
    (void)widget; // ununsed
    (void)event; // ununsed
    (void)allow_passthrough; // ununsed
}

static void base_on_hover_on(SDLNW_Widget* widget, SDLNW_Event_MouseMove* event, bool* allow_passthrough) {
    (void)widget;
    (void)event;
    (void)allow_passthrough;
}

static void base_on_hover_off(SDLNW_Widget* widget, SDLNW_Event_MouseMove* event, bool* allow_passthrough) {
    (void)widget;
    (void)event;
    (void)allow_passthrough;
}

static void base_on_key_up(SDLNW_Widget* widget, SDLNW_Event_KeyUp* event, bool* allow_passthrough) {
    (void)widget;
    (void)event;
    (void)allow_passthrough;
}

static void base_on_text_input(SDLNW_Widget* widget, SDLNW_Event_TextInput* event, bool* allow_passthrough) {
    (void)widget;
    (void)event;
    (void)allow_passthrough;
}

static void init_default_vtable(SDLNW_Widget_VTable* table) {
    table->draw = base_draw;
    table->size = base_size;
    table->click = base_click;
    table->destroy = base_destroy;
    table->appropriate_cursor = base_appropriate_cursor;
    table->get_requested_size = base_get_requested_size;
    table->trickle_down_event = base_trickle_down_event;
    table->mouse_scroll = base_mouse_scroll;
    table->drag = base_drag;
    table->on_hover_on = base_on_hover_on;
    table->on_hover_off = base_on_hover_off;
    table->on_key_up = base_on_key_up;
    table->on_text_input = base_on_text_input;
}

// init default widget
SDLNW_Widget* create_default_widget(void) {
    SDLNW_Widget* p = __sdlnw_malloc(sizeof(SDLNW_Widget));
    
    *p = (SDLNW_Widget){0};

    init_default_vtable(&p->vtable);

    return p;
}

bool is_point_within_rect(int x, int y, const SDL_Rect* rect) {
    return x >= rect->x && x < rect->x + rect->w &&
        y >= rect->y && y < rect->y + rect->h;
}

void _sdlnw_copy_null_terminated(SDLNW_Widget** src_null_terminted, SDLNW_Widget*** target, size_t* target_length) {
    size_t len = 0;
    const size_t max = 2000;

    while(src_null_terminted[len] != NULL && len < max) {
        len += 1;
    }

    if (len == max) {
        // Too long, might not have been array
        *target = NULL;
        *target_length = 0;
        return;
    }

    SDLNW_Widget** p = __sdlnw_malloc(len * sizeof(SDLNW_Widget*));
    for (size_t i = 0; i < len; i++) {
        p[i] = src_null_terminted[i];
    }

    *target = p;
    *target_length = len;
}

typedef struct MemEntry {
    void *ptr;
    size_t size;
    const char *file;
    int line;
    struct MemEntry *next;
} MemEntry;
static MemEntry *mem_list = NULL;

static void add_entry(void *ptr, size_t size, const char *file, int line) {
    MemEntry *entry = malloc(sizeof(MemEntry));
    entry->ptr = ptr;
    entry->size = size;
    entry->file = file;
    entry->line = line;
    entry->next = mem_list;
    mem_list = entry;
}

static void remove_entry(void *ptr) {
    MemEntry **curr = &mem_list;
    while (*curr) {
        if ((*curr)->ptr == ptr) {
            MemEntry *to_free = *curr;
            *curr = (*curr)->next;
            free(to_free);
            return;
        }
        curr = &((*curr)->next);
    }
}

void* __sdlnw_debug_malloc(size_t size, const char *file, int line) {
    void *ptr = malloc(size);
    if (ptr) add_entry(ptr, size, file, line);
    return ptr;
}

void* __sdlnw_debug_realloc(void *ptr, size_t size, const char *file, int line) {
    void* new_ptr = realloc(ptr, size);
    if (new_ptr) {
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wuse-after-free"
        remove_entry(ptr);
        #pragma GCC diagnostic pop
        add_entry(new_ptr, size, file, line);
    }
    return new_ptr;
}

void __sdlnw_debug_free(void *ptr) {
    remove_entry(ptr);
    free(ptr);
}

void SDLNW_debug_report_leaks(void) {
    MemEntry *curr = mem_list;
    while (curr) {
        printf("Memory leak: %p of %zu bytes allocated at %s:%d\n",
               curr->ptr, curr->size, curr->file, curr->line);
        curr = curr->next;
    }
}
