#include <munit.h>
#include "SDL_mouse.h"
#include "SDL_pixels.h"
#include "SDL_render.h"
#include "SDL_video.h"
#include "src/lib/include/SDLNW.h"
#include "test_misc.h"

#define RED (SDL_Color) {.r = 0XFF, .a = 0xFF}
#define GREEN (SDL_Color) {.g = 0XFF, .a = 0xFF}

static SDLNW_Widget* compose_Composite_draw(SDLNW_Widget* parent, void* data) {
    (void)parent;
    (void)data;
    SDLNW_Widget* w = SDLNW_CreateEmptyWidget();
    SDLNW_SetWidgetBackground(w, SDLNW_CreateSolidBackground(RED));
    return w;
}

static MunitResult test_Composite_draw(const MunitParameter params[], void *user_data_or_fixture) {
    (void)params;
    (void)user_data_or_fixture;

    SDLNW_Widget* w = SDLNW_CreateCompositeWidget(NULL, compose_Composite_draw);
    SDL_Rect size = {0, 0, 300, 300};

    double diff = compareImages(RENDERED_DIR "test_Composite_draw.png", TEST_DIR "test_Composite_draw.png", w, &size);
    munit_assert_double(diff, <=, 1.0);

    SDLNW_DestroyWidget(w);

    return MUNIT_OK;
}

static SDL_SystemCursor get_cursor(SDLNW_Widget* widget, int x, int y) {
    (void)widget;
    (void)x;
    (void)y;
    return SDL_SYSTEM_CURSOR_CROSSHAIR;
}

static SDLNW_Widget* compose_Composite_cursor(SDLNW_Widget* parent, void* data) {
    (void)parent;
    (void)data;
    SDLNW_Widget* w = SDLNW_CreateEmptyWidget();
    w->vtable.appropriate_cursor = get_cursor;
    SDLNW_SetWidgetBackground(w, SDLNW_CreateSolidBackground(RED));
    return w;
}

static MunitResult test_Composite_cursor(const MunitParameter params[], void *user_data_or_fixture) {
    (void)params;
    (void)user_data_or_fixture;

    SDLNW_Widget* w = SDLNW_CreateCompositeWidget(NULL, compose_Composite_cursor);
    SDL_Rect size = {0, 0, 300, 300};

    SDLNW_SetWidgetNetSize(w, &size);
    dummy_render(w);
    SDL_SystemCursor c = SDLNW_GetAppropriateCursorForWidget(w, 150, 150);

    munit_assert_int(c, ==, SDL_SYSTEM_CURSOR_CROSSHAIR);

    SDLNW_DestroyWidget(w);

    return MUNIT_OK;
}

static SDLNW_Widget* compose_Composite_recompose(SDLNW_Widget* parent, void* data) {
    (void)parent;
    SDLNW_Widget* w = SDLNW_CreateEmptyWidget();
    int* counter = data;

    w->vtable.appropriate_cursor = get_cursor;
    if (*counter % 2) {
        SDLNW_SetWidgetBackground(w, SDLNW_CreateSolidBackground(RED));
    }
    else {
        SDLNW_SetWidgetBackground(w, SDLNW_CreateSolidBackground(GREEN));
    }

    return w;
}

static MunitResult test_Composite_recompose(const MunitParameter params[], void *user_data_or_fixture) {
    (void)params;
    (void)user_data_or_fixture;

    int counter = 0;

    SDLNW_Widget* w = SDLNW_CreateCompositeWidget(&counter, compose_Composite_recompose);
    SDL_Rect size = {0, 0, 300, 300};

    double diff = compareImages(RENDERED_DIR "test_Composite_recompose_a.png", TEST_DIR "test_Composite_recompose_a.png", w, &size);
    munit_assert_double(diff, <=, 1.0);

    counter++;
    SDLNW_RecomposeWidget(w);

    diff = compareImages(RENDERED_DIR "test_Composite_recompose_b.png", TEST_DIR "test_Composite_recompose_b.png", w, &size);
    munit_assert_double(diff, <=, 1.0);

    SDLNW_DestroyWidget(w);

    return MUNIT_OK;
}

SDLNW_SizeResponse get_size(SDLNW_Widget *w, SDLNW_SizeRequest request) {
    (void)w;

    return (SDLNW_SizeResponse) {
        .width = {
            .pixels = request.total_pixels_avaliable_width / 2
        },
        .height = {
            .pixels = request.total_pixels_avaliable_height / 2
        }
    };
}

static SDLNW_Widget* compose_Composite_request_size(SDLNW_Widget* parent, void* data) {
    (void)parent;
    (void)data;

    SDLNW_Widget* w = SDLNW_CreateEmptyWidget();
    w->vtable.get_requested_size = get_size;

    return w;
}

static MunitResult test_Composite_request_size(const MunitParameter params[], void *user_data_or_fixture) {
    (void)params;
    (void)user_data_or_fixture;

    SDLNW_Widget* w = SDLNW_CreateCompositeWidget(NULL, compose_Composite_request_size);

    SDLNW_SizeResponse resp = SDLNW_GetWidgetRequestedSize(w, (SDLNW_SizeRequest) {
        .total_pixels_avaliable_width = 300,
        .total_pixels_avaliable_height = 300
    });

    munit_assert_int(resp.width.pixels, ==, 150);
    munit_assert_int(resp.height.pixels, ==, 150);

    SDLNW_DestroyWidget(w);

    return MUNIT_OK;
}

static void click(SDLNW_Widget* widget, SDLNW_ClickEvent* event, bool* allow_passthrough) {
    (void)event;
    (void)allow_passthrough;
    int* counter = widget->data;
    (*counter)++;
}

static SDLNW_Widget* compose_Composite_trickle_event(SDLNW_Widget* parent, void* data) {
    (void)parent;
    SDLNW_Widget* w = SDLNW_CreateEmptyWidget();

    w->data = data;
    w->vtable.click = click;

    return w;
}

static MunitResult test_Composite_trickle_event(const MunitParameter params[], void *user_data_or_fixture) {
    (void)params;
    (void)user_data_or_fixture;

    int* counter = malloc(sizeof(int));
    *counter = 0;

    SDLNW_Widget* w = SDLNW_CreateCompositeWidget(counter, compose_Composite_trickle_event);
    SDL_Rect size = {0, 0, 300, 300};

    SDLNW_SetWidgetNetSize(w, &size);
    dummy_render(w);
    SDLNW_ClickWidget(w, 150, 150, 1);

    munit_assert_int(*counter, ==, 1);

    SDLNW_DestroyWidget(w);

    return MUNIT_OK;
}

static MunitTest tests[] = {
    BUILD_TEST(test_Composite_draw),
    BUILD_TEST(test_Composite_cursor),
    BUILD_TEST(test_Composite_recompose),
    BUILD_TEST(test_Composite_request_size),
    BUILD_TEST(test_Composite_trickle_event),
    {0}
};

const MunitSuite composite_suite = {
    .prefix = "Composite ",
    .iterations = 1,
    .options = MUNIT_SUITE_OPTION_NONE,
    .suites = NULL,
    .tests = tests
};
