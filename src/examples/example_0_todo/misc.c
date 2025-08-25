#include "misc.h"
#include "SDLNW.h"
#include <string.h>
#include <assert.h>

SDLNW_Font* FONT = NULL;

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

    SDLNW_TextController_init(&appstate.add_item_text_controller);

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

void AppState_destroy(AppState* appstate) {
    free(appstate->items);
    SDLNW_TextController_destroy(&appstate->add_item_text_controller);
    *appstate = (AppState){0};
}
