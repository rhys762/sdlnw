#include "internal_helpers.h"

#include <assert.h>

#include "SDLNW.h"

void SDLNW_TextController_init(SDLNW_TextController* tc) {
    tc->text_capacity = 20;
    tc->text_len = 1;
    tc->text = __sdlnw_malloc(sizeof(char) * tc->text_capacity);
    tc->text[0] = '\0';

    tc->listeners_capacity = 2;
    tc->listeners_len = 0;
    tc->listeners = __sdlnw_malloc(sizeof(SDLNW_TextControllerChangeListener*) * tc->listeners_capacity);
}

void SDLNW_TextController_destroy(SDLNW_TextController* tc) {
    for (size_t i = 0; i < tc->listeners_len; i++) {
        SDLNW_TextControllerChangeListener_Destroy(tc->listeners[i]);
    }

    tc->text_capacity = 0;
    tc->text_len = 0;
    __sdlnw_free(tc->text);
    tc->text = NULL;

    tc->listeners_capacity = 0;
    tc->listeners_len = 0;
    __sdlnw_free(tc->listeners);
    tc->listeners = NULL;
}

static void call_listeners(SDLNW_TextController* tc) {
    for (size_t i = 0; i < tc->listeners_len; i++) {
        SDLNW_TextControllerChangeListener* tcl = tc->listeners[i];
        tcl->callback(tcl->data, tc->text);
    }
}

void SDLNW_TextController_insert(SDLNW_TextController* tc, char c, size_t idx) {
    if (idx >= tc->text_len) {
        // error
        return;
    }

    if (tc->text_len == tc->text_capacity) {
        tc->text_capacity *= 2;
        tc->text = __sdlnw_realloc(tc->text, tc->text_capacity * sizeof(char));
        assert(tc->text != NULL);
    }

    for (size_t i = tc->text_len; i > idx ; i--) {
        tc->text[i] = tc->text[i - 1];
    }

    tc->text[idx] = c;

    tc->text_len += 1;

    call_listeners(tc);
}

void SDLNW_TextController_remove(SDLNW_TextController* tc, size_t idx) {
    if (tc->text_len < 2) {
        return;
    }

    if (idx > (tc->text_len - 2)) {
        return;
    }

    for (size_t i = idx; i < (tc->text_len - 1); i++) {
        tc->text[i] = tc->text[i+1];
    }
    tc->text_len -= 1;

    call_listeners(tc);
}

void SDLNW_TextController_add_change_listener(SDLNW_TextController* tc, SDLNW_TextControllerChangeListener* tccl) {
    if (tc->listeners_len == tc->listeners_capacity) {
        tc->listeners_capacity *= 2;
        tc->listeners = __sdlnw_realloc(tc->listeners, sizeof(SDLNW_TextControllerChangeListener*) * tc->listeners_capacity);
    }

    tc->listeners[tc->listeners_len] = tccl;
    tc->listeners_len += 1;
}

bool SDLNW_TextController_remove_change_listener(SDLNW_TextController* tc, SDLNW_TextControllerChangeListener* tccl) {
    for (size_t i = 0; i < tc->listeners_len; i++) {
        if (tc->listeners[i] == tccl) {
            tc->listeners_len--;

            for (size_t j = i; j < tc->listeners_len; j++) {
                tc->listeners[j] = tc->listeners[j + 1];
            }

            return true;
        }
    }
    return false;
}

void SDLNW_TextController_set_value(SDLNW_TextController* tc, const char* text) {
    size_t required_len = strlen(text) + 1;

    if (tc->text_capacity <= required_len) {
        while (tc->text_capacity <= required_len) {
            tc->text_capacity *= 2;
        }

        tc->text = __sdlnw_realloc(tc->text, sizeof(char) * tc->text_capacity);
    }

    tc->text_len = (size_t)required_len;
    for (size_t i = 0; i < tc->text_len; i++) {
        tc->text[i] = text[i];
    }
    call_listeners(tc);
}

char* SDLNW_TextController_get_value(const SDLNW_TextController* tc) {
    return tc->text;
}
