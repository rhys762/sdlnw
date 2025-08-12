#include <munit.h>
#include "src/lib/include/SDLNW.h"


static MunitResult test_SDLNW_TextController_init(const MunitParameter params[], void *user_data_or_fixture) {
    (void)params;
    (void)user_data_or_fixture;

    SDLNW_TextController tc;
    SDLNW_TextController_init(&tc);

    munit_assert_char(tc.text[0], ==, '\0');

    SDLNW_TextController_destroy(&tc);

    return MUNIT_OK;
}

static MunitResult test_SDLNW_TextController_destroy(const MunitParameter params[], void *user_data_or_fixture) {
    (void)params;
    (void)user_data_or_fixture;

    SDLNW_TextController tc;
    SDLNW_TextController_init(&tc);

    munit_assert_char(tc.text[0], ==, '\0');

    SDLNW_TextController_destroy(&tc);
    munit_assert_ptr(tc.text, ==, NULL);

    return MUNIT_OK;
}

static MunitResult test_SDLNW_TextController_insert(const MunitParameter params[], void *user_data_or_fixture) {
    (void)params;
    (void)user_data_or_fixture;

    SDLNW_TextController tc;
    SDLNW_TextController_init(&tc);

    munit_assert_int(strcmp(tc.text, "\0"), ==, 0);

    // should ignore, after end
    SDLNW_TextController_insert(&tc, 'z', 1);
    munit_assert_int(strcmp(tc.text, "\0"), ==, 0);

    SDLNW_TextController_insert(&tc, 'a', 0);
    munit_assert_int(strcmp(tc.text, "a\0"), ==, 0);
    munit_assert_char(tc.text[1], ==, '\0');

    SDLNW_TextController_insert(&tc, 'b', 0);
    munit_assert_int(strcmp(tc.text, "ba\0"), ==, 0);

    SDLNW_TextController_insert(&tc, 'c', 2);
    munit_assert_int(strcmp(tc.text, "bac\0"), ==, 0);

    SDLNW_TextController_destroy(&tc);

    return MUNIT_OK;
}

static MunitResult test_SDLNW_TextController_remove(const MunitParameter params[], void *user_data_or_fixture) {
    (void)params;
    (void)user_data_or_fixture;

    SDLNW_TextController tc;
    SDLNW_TextController_init(&tc);

    SDLNW_TextController_insert(&tc, 'a', 0);
    SDLNW_TextController_insert(&tc, 'b', 1);
    SDLNW_TextController_insert(&tc, 'c', 2);
    
    munit_assert_int(strcmp(tc.text, "abc\0"), ==, 0);

    SDLNW_TextController_remove(&tc, 10);
    munit_assert_int(strcmp(tc.text, "abc\0"), ==, 0);

    SDLNW_TextController_remove(&tc, 0);
    munit_assert_int(strcmp(tc.text, "bc\0"), ==, 0);
    SDLNW_TextController_remove(&tc, 1);
    munit_assert_int(strcmp(tc.text, "b\0"), ==, 0);
    SDLNW_TextController_remove(&tc, 0);
    munit_assert_int(strcmp(tc.text, "\0"), ==, 0);
    SDLNW_TextController_remove(&tc, 0);
    munit_assert_int(strcmp(tc.text, "\0"), ==, 0);
    SDLNW_TextController_remove(&tc, 0);
    munit_assert_int(strcmp(tc.text, "\0"), ==, 0);
    SDLNW_TextController_remove(&tc, 0);
    munit_assert_int(strcmp(tc.text, "\0"), ==, 0);

    SDLNW_TextController_destroy(&tc);

    return MUNIT_OK;
}

static MunitResult test_SDLNW_TextController_add_change_listener(const MunitParameter params[], void *user_data_or_fixture) {
    (void)params;
    (void)user_data_or_fixture;

    SDLNW_TextController tc;
    char buffer[30];

    SDLNW_TextController_init(&tc);

    SDLNW_TextController_add_change_listener(&tc, SDLNW_CreateTextControllerChangeListener(buffer, strcpy, NULL));

    // write some characters and remove some characters, check each time buffer is updated
    SDLNW_TextController_insert(&tc, 'a', 0);
    munit_assert_int(strcmp(buffer, "a\0"), ==, 0);
    SDLNW_TextController_insert(&tc, 'b', 0);
    munit_assert_int(strcmp(buffer, "ba\0"), ==, 0);
    SDLNW_TextController_remove(&tc, 0);
    munit_assert_int(strcmp(buffer, "a\0"), ==, 0);

    SDLNW_TextController_destroy(&tc);

    return MUNIT_OK;
}

static MunitResult test_SDLNW_TextController_set_value(const MunitParameter params[], void *user_data_or_fixture) {
    (void)params;
    (void)user_data_or_fixture;

    SDLNW_TextController tc;

    SDLNW_TextController_init(&tc);

    SDLNW_TextController_set_value(&tc, "Hello world!");
    munit_assert_int(strcmp(tc.text, "Hello world!"), ==, 0);

    SDLNW_TextController_set_value(&tc, "I am set with SDLNW_TextController_set_value!");
    munit_assert_int(strcmp(tc.text, "I am set with SDLNW_TextController_set_value!"), ==, 0);

    SDLNW_TextController_destroy(&tc);

    return MUNIT_OK;
}

// trivial but for completions sake
static MunitResult test_SDLNW_TextController_get_value(const MunitParameter params[], void *user_data_or_fixture) {
    (void)params;
    (void)user_data_or_fixture;

    SDLNW_TextController tc;

    SDLNW_TextController_init(&tc);

    munit_assert_ptr(SDLNW_TextController_get_value(&tc), ==, tc.text);

    SDLNW_TextController_destroy(&tc);

    return MUNIT_OK;
}

#define BUILD_TEST(FUNC) { .name = #FUNC, .test = FUNC}

static MunitTest tests[] = {
    BUILD_TEST(test_SDLNW_TextController_init),
    BUILD_TEST(test_SDLNW_TextController_destroy),
    BUILD_TEST(test_SDLNW_TextController_insert),
    BUILD_TEST(test_SDLNW_TextController_remove),
    BUILD_TEST(test_SDLNW_TextController_add_change_listener),
    BUILD_TEST(test_SDLNW_TextController_set_value),
    BUILD_TEST(test_SDLNW_TextController_get_value),
    {0}
};

const MunitSuite text_controller_suite = {
    .prefix = "TextController ",
    .iterations = 1,
    .options = MUNIT_SUITE_OPTION_NONE,
    .suites = NULL,
    .tests = tests
};
