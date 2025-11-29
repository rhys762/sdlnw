#include <munit.h>
#include "SDLNW.h"
#include "SDL_mouse.h"
#include "test_misc.h"

static MunitResult test_ZStack_render(const MunitParameter params[], void *user_data_or_fixture) {
    (void)params;
    (void)user_data_or_fixture;

    SDLNW_Widget* a = SDLNW_CreateSizedBoxWidget(SDLNW_CreateEmptyWidget(), (SDLNW_SizedBoxWidgetOptions) {
        .height_shares = 1,
        .width_shares = 1
    });
    SDLNW_SetWidgetBackground(a, SDLNW_CreateSolidBackground((SDL_Color) {.r = 0xFF, .a = 0xFF}));

    SDLNW_Widget* b_inner = SDLNW_CreateEmptyWidget();
    SDLNW_SetWidgetBackground(b_inner, SDLNW_CreateSolidBackground((SDL_Color) {.g = 0xFF, .a = 0xFF}));
    SDLNW_SetWidgetPadding(b_inner, (SDLNW_Insets) {
        .top = 25,
        .right = 25,
        .bottom = 25,
        .left = 25,
    });
    SDLNW_Widget* b = SDLNW_CreateCentreWidget(b_inner);

    SDLNW_Widget* arr[] = {a, b, NULL};

    SDLNW_Widget* zstack = SDLNW_CreateZStackWidget(arr);
    SDLNW_SetWidgetBackground(zstack, SDLNW_CreateSolidBackground((SDL_Color) {.a = 0xFF}));

    SDL_Rect size = {0, 0, 300, 300};

    double diff = compareImages(RENDERED_DIR "test_ZStack_render.png", TEST_DIR "test_ZStack_render.png", zstack, &size);
    munit_assert_double(diff, <=, 1.0);

    SDLNW_DestroyWidget(zstack);

    return MUNIT_OK;
}

static SDL_SystemCursor cursor_override(SDLNW_Widget* w, int x, int y) {
    (void)w;
    (void)x;
    (void)y;

    return SDL_SYSTEM_CURSOR_CROSSHAIR;
}

static MunitResult test_ZStack_cursor(const MunitParameter params[], void *user_data_or_fixture) {
    (void)params;
    (void)user_data_or_fixture;

    SDLNW_Widget* a = SDLNW_CreateSizedBoxWidget(SDLNW_CreateEmptyWidget(), (SDLNW_SizedBoxWidgetOptions) {
        .height_shares = 1,
        .width_shares = 1
    });
    SDLNW_SetWidgetBackground(a, SDLNW_CreateSolidBackground((SDL_Color) {.r = 0xFF, .a = 0xFF}));

    SDLNW_Widget* b_inner = SDLNW_CreateEmptyWidget();
    SDLNW_SetWidgetBackground(b_inner, SDLNW_CreateSolidBackground((SDL_Color) {.g = 0xFF, .a = 0xFF}));
    SDLNW_SetWidgetPadding(b_inner, (SDLNW_Insets) {
        .top = 25,
        .right = 25,
        .bottom = 25,
        .left = 25,
    });
    SDLNW_Widget* b = SDLNW_CreateCentreWidget(b_inner);

    SDLNW_Widget* arr[] = {a, b, NULL};

    SDLNW_Widget* zstack = SDLNW_CreateZStackWidget(arr);
    SDLNW_SetWidgetBackground(zstack, SDLNW_CreateSolidBackground((SDL_Color) {.a = 0xFF}));
    b_inner->vtable.appropriate_cursor = cursor_override;

    SDL_Rect size = {0, 0, 300, 300};

    SDLNW_SetWidgetNetSize(zstack, &size);
    dummy_render(zstack);

    SDL_SystemCursor cursor;
    cursor = SDLNW_GetAppropriateCursorForWidget(zstack, 1, 1);
    munit_assert_int(cursor, ==, SDL_SYSTEM_CURSOR_ARROW);

    cursor = SDLNW_GetAppropriateCursorForWidget(zstack, 150, 150);
    munit_assert_int(cursor, ==, SDL_SYSTEM_CURSOR_CROSSHAIR);

    SDLNW_DestroyWidget(zstack);

    return MUNIT_OK;
}

static MunitResult test_ZStack_request_size(const MunitParameter params[], void *user_data_or_fixture) {
    (void)params;
    (void)user_data_or_fixture;

    SDLNW_Widget* a = SDLNW_CreateSizedBoxWidget(SDLNW_CreateEmptyWidget(), (SDLNW_SizedBoxWidgetOptions) {
        .height_pixels = 200,
        .width_pixels = 50
    });

    SDLNW_Widget* b = SDLNW_CreateSizedBoxWidget(SDLNW_CreateEmptyWidget(), (SDLNW_SizedBoxWidgetOptions) {
        .height_pixels = 50,
        .width_pixels = 100
    });


    SDLNW_Widget* arr[] = {a, b, NULL};

    SDLNW_Widget* zstack = SDLNW_CreateZStackWidget(arr);

    SDLNW_SizeResponse response = SDLNW_GetWidgetRequestedSize(zstack, (SDLNW_SizeRequest) {
        0
    });

    munit_assert_int(response.width.pixels, ==, 100);
    munit_assert_int(response.width.shares, ==, 0);
    munit_assert_int(response.height.pixels, ==, 200);
    munit_assert_int(response.width.shares, ==, 0);

    SDLNW_DestroyWidget(zstack);

    return MUNIT_OK;
}

void click_a(void* data, int x, int y, bool* allow_passthrough) {
    (void)x;
    (void)y;
    int* i = data;
    *i = 1;
    *allow_passthrough = false;
}

void click_b(void* data, int x, int y, bool* allow_passthrough) {
    (void)x;
    (void)y;
    int* i = data;
    *i = 2;
    *allow_passthrough = false;
}

static MunitResult test_ZStack_request_click(const MunitParameter params[], void *user_data_or_fixture) {
    (void)params;
    (void)user_data_or_fixture;

    SDLNW_Widget* a = SDLNW_CreateSizedBoxWidget(SDLNW_CreateEmptyWidget(), (SDLNW_SizedBoxWidgetOptions) {
        .height_shares = 1,
        .width_shares = 1
    });
    SDLNW_SetWidgetBackground(a, SDLNW_CreateSolidBackground((SDL_Color) {.r = 0xFF, .a = 0xFF}));

    SDLNW_Widget* b_inner = SDLNW_CreateEmptyWidget();
    SDLNW_SetWidgetBackground(b_inner, SDLNW_CreateSolidBackground((SDL_Color) {.g = 0xFF, .a = 0xFF}));
    SDLNW_SetWidgetPadding(b_inner, (SDLNW_Insets) {
        .top = 25,
        .right = 25,
        .bottom = 25,
        .left = 25,
    });

    int i = 0;

    SDLNW_Widget* b = SDLNW_CreateCentreWidget(SDLNW_CreateGestureDetectorWidget(b_inner, (SDLNW_GestureDetectorWidgetOptions) {
        .on_click = click_b,
        .data = &i
    }));


    SDLNW_Widget* arr[] = { SDLNW_CreateGestureDetectorWidget(a, (SDLNW_GestureDetectorWidgetOptions) {
        .on_click = click_a,
        .data = &i
    }), b, NULL};

    SDLNW_Widget* zstack = SDLNW_CreateZStackWidget(arr);
    SDLNW_SetWidgetBackground(zstack, SDLNW_CreateSolidBackground((SDL_Color) {.a = 0xFF}));
    b_inner->vtable.appropriate_cursor = cursor_override;

    SDL_Rect size = {0, 0, 300, 300};

    SDLNW_SetWidgetNetSize(zstack, &size);
    dummy_render(zstack);

    SDLNW_ClickWidget(zstack, 0, 0, 1);
    munit_assert_int(i, ==, 1);

    SDLNW_ClickWidget(zstack, 150, 150, 1);
    munit_assert_int(i, ==, 2);

    SDLNW_DestroyWidget(zstack);

    return MUNIT_OK;
}

static MunitTest tests[] = {
    BUILD_TEST(test_ZStack_render),
    BUILD_TEST(test_ZStack_cursor),
    BUILD_TEST(test_ZStack_request_size),
    BUILD_TEST(test_ZStack_request_click),
    {0}
};

const MunitSuite zstack_suite = {
    .prefix = "ZStack ",
    .iterations = 1,
    .options = MUNIT_SUITE_OPTION_NONE,
    .suites = NULL,
    .tests = tests
};
