
#include <stddef.h>
#include <assert.h>
#include <math.h>

#include "SDLNWInternal.h"

// for f(x)
typedef struct {
    void* data;
    // compute prioity(x) for each node, highest wins
    int (*priority)(void* data, int x);
    // winner is used to compute f(x)
    int (*compute)(void* data, int x);
} __sdlnw_int_spline_node;

struct __sdlnw_int_spline_struct {
    __sdlnw_int_spline_node* nodes;
    size_t nodes_len;
    size_t nodes_cap;

};

__sdlnw_int_spline* __sdlnw_int_spline_create(void) {
    __sdlnw_int_spline* p = __sdlnw_malloc(sizeof(__sdlnw_int_spline));

    *p = (__sdlnw_int_spline) {0};
    p->nodes_cap = 2;
    p->nodes = __sdlnw_malloc(p->nodes_cap * sizeof(__sdlnw_int_spline_node));

    return p;
}

void __sdlnw_int_spline_destroy(__sdlnw_int_spline* spline) {
    for (size_t i = 0; i < spline->nodes_len; i++) {
        __sdlnw_free(spline->nodes[i].data);
    }

    __sdlnw_free(spline->nodes);
    *spline = (__sdlnw_int_spline){0};
    __sdlnw_free(spline);
}

void __sdlnw_int_spline_add_node(__sdlnw_int_spline* spline, void* data, int (*priority)(void* data, int x), int (*compute)(void* data, int x)) {
    if (spline->nodes_len >= spline->nodes_cap) {
        spline->nodes_cap *= 2;
        spline->nodes = __sdlnw_realloc(spline->nodes, spline->nodes_cap * sizeof(__sdlnw_int_spline_node));
    }

    spline->nodes[spline->nodes_len] = (__sdlnw_int_spline_node) {
        .data = data,
        .priority = priority,
        .compute = compute
    };

    spline->nodes_len += 1;
}

int __sdlnw_int_spline_compute(__sdlnw_int_spline* spline, int x) {
    assert(spline->nodes_len > 0);

    int highest_index = 0;
    int highest_priority = spline->nodes[0].priority(spline->nodes[0].data, x);

    for (size_t i = 1; i < spline->nodes_len; i++) {
        __sdlnw_int_spline_node* n = &spline->nodes[i];
        int p = n->priority(n->data, x);
        if (p > highest_priority) {
            highest_priority = p;
            highest_index = i;
        }
    }

    __sdlnw_int_spline_node* n = &spline->nodes[highest_index];
    return n->compute(n->data, x);
}

// stolen from math.h
#define M_PI 3.14159265358979323846

enum CORNER {
    TOP_LEFT,
    TOP_RIGHT,
    BOTTOM_LEFT,
    BOTTOM_RIGHT
};

struct corner_info {
    int radius;
    SDL_Rect size;
    enum CORNER corner;
};

int priority_default(void* data, int x) {
    (void)data;
    (void)x;
    return 1;
}

int compute_top_line(void* data, int x) {
    (void)data;
    (void)x;
    return 0;
}

int compute_bottom_line(void* data, int x) {
    (void)x;
    int y = *(int*)data;

    return y;
}

int priority_corner(void* data, int x) {
    struct corner_info* ci = data;
    int radius = ci->radius;

    switch (ci->corner) {
        case TOP_LEFT:
        case BOTTOM_LEFT:
            if (x <= radius) {
                return 2;
            }
            break;
        case TOP_RIGHT:
        case BOTTOM_RIGHT:
            if ((ci->size.w - x) <= radius) {
                return 2;
            }
            break;
    }

    return 0;
}


int compute_corner(void* data, int x) {
    struct corner_info* ci = data;

    double centre_x = 0.0, centre_y = 0.0;
    double radian_shift = 0;

    double radius = (double)ci->radius;

    switch (ci->corner) {
        case TOP_LEFT:
            centre_x = radius;
            centre_y = radius;
            radian_shift = M_PI;
            break;
        case TOP_RIGHT:
            centre_x = (double)ci->size.w - radius - 1;
            centre_y = radius;
            radian_shift = M_PI;
            break;
        case BOTTOM_LEFT:
            centre_x = radius;
            centre_y = (double)ci->size.h - radius - 1;
            radian_shift = 0.0;
            break;
        case BOTTOM_RIGHT:
            centre_x = (double)ci->size.w - radius - 1;
            centre_y = (double)ci->size.h - radius - 1;
            radian_shift = 0.0;
          break;
    }
    double dx = (double)x;


    // x = radius * cos(theta) + centre_x
    double radians = acos((dx - centre_x)/radius);
    radians -= radian_shift;

    double y = radius * sin(radians) + centre_y;
    return (int)round(y);
}

static __sdlnw_int_spline* __sdlnw_int_spline_create_rounded_box_top_line(int width, int height, const SDLNW_CornerRadius* radius) {
    SDL_Rect size = {
        .w = width,
        .h = height
    };

    __sdlnw_int_spline* top_line = __sdlnw_int_spline_create();
    __sdlnw_int_spline_add_node(top_line, NULL, priority_default, compute_top_line);

    if (radius->top_left) {
        struct corner_info* top_left = __sdlnw_malloc(sizeof(struct corner_info));

        *top_left = (struct corner_info) {
            .radius = radius->top_left,
            .corner = TOP_LEFT,
            .size = size
        };

        __sdlnw_int_spline_add_node(top_line, top_left, priority_corner, compute_corner);
    }

    if (radius->top_right) {
        struct corner_info* top_right = __sdlnw_malloc(sizeof(struct corner_info));

        *top_right = (struct corner_info) {
            .radius = radius->top_right,
            .corner = TOP_RIGHT,
            .size = size
        };

        __sdlnw_int_spline_add_node(top_line, top_right, priority_corner, compute_corner);
    }

    return top_line;
}

static __sdlnw_int_spline* __sdlnw_int_spline_create_rounded_box_bottom_line(int width, int height, const SDLNW_CornerRadius* radius) {
    SDL_Rect size = {
        .w = width,
        .h = height
    };

    __sdlnw_int_spline* bottom_line = __sdlnw_int_spline_create();

    int* h = __sdlnw_malloc(sizeof(int));
    *h = height;
    __sdlnw_int_spline_add_node(bottom_line, h, priority_default, compute_bottom_line);

    if (radius->bottom_left) {
        struct corner_info* bottom_left = __sdlnw_malloc(sizeof(struct corner_info));

        *bottom_left = (struct corner_info) {
            .radius = radius->bottom_left,
            .corner = BOTTOM_LEFT,
            .size = size
        };

        __sdlnw_int_spline_add_node(bottom_line, bottom_left, priority_corner, compute_corner);
    }

    if (radius->bottom_right) {
        struct corner_info* bottom_right = __sdlnw_malloc(sizeof(struct corner_info));

        *bottom_right = (struct corner_info) {
            .radius = radius->bottom_right,
            .corner = BOTTOM_RIGHT,
            .size = size
        };

        __sdlnw_int_spline_add_node(bottom_line, bottom_right, priority_corner, compute_corner);
    }

    return bottom_line;
}

struct __sdlnw_int_spline_rounded_box_struct {
    __sdlnw_int_spline* top_line;
    __sdlnw_int_spline* bottom_line;
};

__sdlnw_int_spline_rounded_box* __sdlnw_int_spline_rounded_box_create(int width, int height, const SDLNW_CornerRadius* radius) {
    __sdlnw_int_spline_rounded_box* box = __sdlnw_malloc(sizeof(__sdlnw_int_spline_rounded_box));
    *box = (__sdlnw_int_spline_rounded_box) {0};

    box->top_line = __sdlnw_int_spline_create_rounded_box_top_line(width, height, radius);
    box->bottom_line = __sdlnw_int_spline_create_rounded_box_bottom_line(width, height, radius);

    return box;
}

void __sdlnw_int_spline_rounded_box_destroy(__sdlnw_int_spline_rounded_box* box) {
    __sdlnw_int_spline_destroy(box->top_line);
    __sdlnw_int_spline_destroy(box->bottom_line);
    __sdlnw_free(box);
}

bool __sdlnw_int_spline_rounded_box_within_bounds(const __sdlnw_int_spline_rounded_box* box, int x, int y) {
    return y >= __sdlnw_int_spline_compute(box->top_line, x)  && y < __sdlnw_int_spline_compute(box->bottom_line, x);
}
