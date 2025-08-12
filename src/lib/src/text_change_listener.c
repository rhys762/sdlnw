#include "internal_helpers.h"

#include "SDLNW.h"

SDLNW_TextControllerChangeListener* SDLNW_CreateTextControllerChangeListener(void* data, void (*callback) (void*,char*), void (*free_data)(void*)) {
    SDLNW_TextControllerChangeListener* p = __sdlnw_malloc(sizeof(SDLNW_TextControllerChangeListener));

    *p = (SDLNW_TextControllerChangeListener){
        .data = data,
        .callback = callback,
        .free_data = free_data
    };

    return p;
}

void SDLNW_TextControllerChangeListener_Destroy(SDLNW_TextControllerChangeListener* tccl) {
    if (tccl->free_data) {
        tccl->free_data(tccl->data);
    }

    __sdlnw_free(tccl);
}