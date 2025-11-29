#include <munit.h>
#include "src/lib/include/SDLNW.h"
#include "test_misc.h"
#include "SDLNWInternal.h"

static MunitResult test_DrawBorder(const MunitParameter params[], void *user_data_or_fixture) {
    (void)params;
    (void)user_data_or_fixture;

    const SDL_Rect size = {0, 0, 300, 300};

    SDLNW_Widget* w = SDLNW_CreateEmptyWidget();

    SDLNW_SetWidgetBackground(w, SDLNW_CreateSolidBackground((SDL_Color) {
        .r = 0xFF,
        .a = 0xFF
    }));

    SDLNW_SetWidgetBorder(w, SDLNW_CreateSolidBorder(2, (SDL_Color) {
        .g = 0xFF,
        .a = 0xFF
    }));

    // test without radius

    double diff = compareImages(RENDERED_DIR "test_Border_render_without_radius.png", TEST_DIR "test_Border_render_without_radius.png", w, &size);
    munit_assert_double(diff, <=, 1.0);

    // test with radius
    SDLNW_SetWidgetCornerRadius(w, (SDLNW_CornerRadius) {
        .top_left = 5,
        .top_right = 5,
        .bottom_left = 5,
        .bottom_right = 5
    });

    diff = compareImages(RENDERED_DIR "test_Border_render_with_radius.png", TEST_DIR "test_Border_render_with_radius.png", w, &size);
    munit_assert_double(diff, <=, 1.0);

    SDLNW_DestroyWidget(w);

    return MUNIT_OK;
}

static MunitTest tests[] = {
    BUILD_TEST(test_DrawBorder),
    {0}
};

const MunitSuite border_suite = {
    .prefix = "Border ",
    .iterations = 1,
    .options = MUNIT_SUITE_OPTION_NONE,
    .suites = NULL,
    .tests = tests
};
