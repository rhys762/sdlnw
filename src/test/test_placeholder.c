#include <munit.h>
#include "src/lib/include/SDLNW.h"
#include "test_misc.h"

static MunitResult test_Placeholder_size(const MunitParameter params[], void *user_data_or_fixture) {
    (void)params;
    (void)user_data_or_fixture;
    SDLNW_Widget* p = SDLNW_CreatePlaceholderWidget();

    // assert its non-zero pixels and shares
    SDLNW_SizeResponse res = SDLNW_GetWidgetRequestedSize(p, (SDLNW_SizeRequest) {
        .total_pixels_avaliable_height = 200,
        .total_pixels_avaliable_width = 200
    });

    munit_assert_int(res.height.pixels, >, 0);
    munit_assert_int(res.height.shares, >, 0);
    munit_assert_int(res.width.pixels, >, 0);
    munit_assert_int(res.width.shares, >, 0);

    SDLNW_DestroyWidget(p);

    return MUNIT_OK;
}

static MunitResult test_Placeholder_draw(const MunitParameter params[], void *user_data_or_fixture) {
    (void)params;
    (void)user_data_or_fixture;

    SDLNW_Widget* p = SDLNW_CreatePlaceholderWidget();

    SDL_Rect size = {0, 0, 300, 300};

    double diff = compareImages(RENDERED_DIR "test_Placeholder_draw.png", TEST_DIR "test_Placeholder_draw.png", p, &size);
    munit_assert_double(diff, <=, 1.0);

    SDLNW_DestroyWidget(p);

    return MUNIT_OK;
}

static MunitTest tests[] = {
    BUILD_TEST(test_Placeholder_size),
    BUILD_TEST(test_Placeholder_draw),
    {0}
};

const MunitSuite placeholder_suite = {
    .prefix = "Placeholder ",
    .iterations = 1,
    .options = MUNIT_SUITE_OPTION_NONE,
    .suites = NULL,
    .tests = tests
};
