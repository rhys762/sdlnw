#include <munit.h>
#include "SDL_mouse.h"
#include "SDL_pixels.h"
#include "src/lib/include/SDLNW.h"
#include "test_misc.h"

static MunitResult test_Column_size(const MunitParameter params[], void *user_data_or_fixture) {
    (void)params;
    (void)user_data_or_fixture;

    SDLNW_Widget* a = SDLNW_CreateSizedBoxWidget(SDLNW_CreateEmptyWidget(), (SDLNW_SizedBoxWidgetOptions) {
        .height_pixels = 10,
        .height_shares = 1,
        .width_pixels = 20,
        .width_shares = 2
    });

    SDLNW_Widget* b = SDLNW_CreateSizedBoxWidget(SDLNW_CreateEmptyWidget(), (SDLNW_SizedBoxWidgetOptions) {
        .height_pixels = 50,
        .height_shares = 5,
        .width_pixels = 80,
        .width_shares = 8
    }) ;

    SDLNW_Widget* arr[] = {a, b, NULL};

    SDLNW_Widget* column = SDLNW_CreateColumnWidget(arr);

    // assert horizontal is max and vertical is sum
    SDLNW_SizeResponse res = SDLNW_GetWidgetRequestedSize(column, (SDLNW_SizeRequest) {
        .total_pixels_avaliable_height = 200,
        .total_pixels_avaliable_width = 200
    });

    munit_assert_int(res.height.pixels, ==, 60);
    munit_assert_int(res.height.shares, ==, 6);
    munit_assert_int(res.width.pixels, ==, 80);
    munit_assert_int(res.width.shares, ==, 8);

    SDLNW_DestroyWidget(column);

    return MUNIT_OK;
}

static MunitResult test_Column_render(const MunitParameter params[], void *user_data_or_fixture) {
    (void)params;
    (void)user_data_or_fixture;

    SDLNW_Widget* a = SDLNW_CreateSizedBoxWidget(SDLNW_CreateEmptyWidget(), (SDLNW_SizedBoxWidgetOptions) {
        .height_shares = 1,
        .width_shares = 1
    });
    SDLNW_SetWidgetBackground(a, SDLNW_CreateSolidBackground((SDL_Color) {.r = 0xFF, .a = 0xFF}));

    SDLNW_Widget* b = SDLNW_CreateSizedBoxWidget(SDLNW_CreateEmptyWidget(), (SDLNW_SizedBoxWidgetOptions) {
        .height_shares = 1,
        .width_shares = 1
    }) ;

    SDLNW_Widget* c = SDLNW_CreateSizedBoxWidget(SDLNW_CreateEmptyWidget(), (SDLNW_SizedBoxWidgetOptions) {
        .height_shares = 1,
        .width_shares = 1
    });

    SDLNW_SetWidgetBackground(c, SDLNW_CreateSolidBackground((SDL_Color) {.g = 0xFF, .a = 0xFF}));

    SDLNW_Widget* arr[] = {a, b, c, NULL};

    SDLNW_Widget* column = SDLNW_CreateColumnWidget(arr);
    SDLNW_SetWidgetBackground(column, SDLNW_CreateSolidBackground((SDL_Color) {.b = 0xFF, .a = 0xFF}));

    SDL_Rect size = {0, 0, 300, 300};

    double diff = compareImages(RENDERED_DIR "test_Column_render.png", TEST_DIR "test_Column_render.png", column, &size);
    munit_assert_double(diff, <=, 1.0);

    SDLNW_DestroyWidget(column);

    return MUNIT_OK;
}

static SDL_SystemCursor hardcoded_cursor(SDLNW_Widget* w, int x, int y) {
    (void)w;
    (void)x;
    (void)y;
    return SDL_SYSTEM_CURSOR_WAIT;
}

static MunitResult test_Column_cursor(const MunitParameter params[], void *user_data_or_fixture) {
    (void)params;
    (void)user_data_or_fixture;

    SDLNW_Widget* a = SDLNW_CreateSizedBoxWidget(SDLNW_CreateEmptyWidget(), (SDLNW_SizedBoxWidgetOptions) {
        .height_shares = 1,
        .width_shares = 1
    });
    a->vtable.appropriate_cursor = hardcoded_cursor;

    SDLNW_Widget* b = SDLNW_CreateSizedBoxWidget(SDLNW_CreateEmptyWidget(), (SDLNW_SizedBoxWidgetOptions) {
        .height_shares = 1,
        .width_shares = 1
    }) ;


    SDLNW_Widget* arr[] = {a, b, NULL};
    SDLNW_Widget* column = SDLNW_CreateColumnWidget(arr);

    SDL_Rect size = {0, 0, 300, 300};
    SDLNW_SetWidgetNetSize(column, &size);

    // should be wait on the top widget and arrow on the bottom
    munit_assert_int(SDLNW_GetAppropriateCursorForWidget(column, 100, 100), ==, SDL_SYSTEM_CURSOR_WAIT);
    munit_assert_int(SDLNW_GetAppropriateCursorForWidget(column, 200, 200), ==, SDL_SYSTEM_CURSOR_ARROW);

    SDLNW_DestroyWidget(column);

    return MUNIT_OK;
}

static void click(SDLNW_Widget* w, SDLNW_ClickEvent* e, bool* allow_passthrough) {
    (void)allow_passthrough;
    int* clicked_at = w->data;
    clicked_at[0] = e->x;
    clicked_at[1] = e->y;
}

static MunitResult test_Column_trickle(const MunitParameter params[], void *user_data_or_fixture) {
    (void)params;
    (void)user_data_or_fixture;

    int clicked_at[] = {0, 0};

    SDLNW_Widget* empty = SDLNW_CreateEmptyWidget();
    empty->data = clicked_at;
    empty->vtable.click = click;

    SDLNW_Widget* a = SDLNW_CreateSizedBoxWidget(empty, (SDLNW_SizedBoxWidgetOptions) {
        .height_shares = 1,
        .width_shares = 1
    });

    SDLNW_Widget* b = SDLNW_CreateSizedBoxWidget(SDLNW_CreateEmptyWidget(), (SDLNW_SizedBoxWidgetOptions) {
        .height_shares = 1,
        .width_shares = 1
    });


    SDLNW_Widget* arr[] = {a, b, NULL};
    SDLNW_Widget* column = SDLNW_CreateColumnWidget(arr);

    SDL_Rect size = {0, 0, 300, 300};
    SDLNW_SetWidgetNetSize(column, &size);

    SDLNW_ClickWidget(column, 50, 100, 1);
    munit_assert_int(clicked_at[0], ==, 50);
    munit_assert_int(clicked_at[1], ==, 100);

    empty->data = NULL;

    SDLNW_DestroyWidget(column);

    return MUNIT_OK;
}

static MunitTest tests[] = {
    BUILD_TEST(test_Column_size),
    BUILD_TEST(test_Column_render),
    BUILD_TEST(test_Column_cursor),
    BUILD_TEST(test_Column_trickle),
    {0}
};

const MunitSuite column_suite = {
    .prefix = "Column ",
    .iterations = 1,
    .options = MUNIT_SUITE_OPTION_NONE,
    .suites = NULL,
    .tests = tests
};
