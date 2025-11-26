#include <munit.h>
#include <string.h>
#include "src/lib/include/SDLNW.h"
#include "test_misc.h"

static MunitResult test_SDLNW_InitTextController(const MunitParameter params[], void *user_data_or_fixture) {
    (void)params;
    (void)user_data_or_fixture;

    SDLNW_TextController tc;
    SDLNW_InitTextController(&tc);

    munit_assert_char(tc.text[0], ==, '\0');

    SDLNW_DestroyTextController(&tc);

    return MUNIT_OK;
}

static MunitResult test_SDLNW_DestroyTextController(const MunitParameter params[], void *user_data_or_fixture) {
    (void)params;
    (void)user_data_or_fixture;

    SDLNW_TextController tc;
    SDLNW_InitTextController(&tc);

    munit_assert_char(tc.text[0], ==, '\0');

    SDLNW_DestroyTextController(&tc);
    munit_assert_ptr(tc.text, ==, NULL);

    return MUNIT_OK;
}

static MunitResult test_SDLNW_TextControllerInsert(const MunitParameter params[], void *user_data_or_fixture) {
    (void)params;
    (void)user_data_or_fixture;

    SDLNW_TextController tc;
    SDLNW_InitTextController(&tc);

    munit_assert_int(strcmp(tc.text, "\0"), ==, 0);

    // should ignore, after end
    SDLNW_TextControllerInsert(&tc, 'z', 1);
    munit_assert_int(strcmp(tc.text, "\0"), ==, 0);

    SDLNW_TextControllerInsert(&tc, 'a', 0);
    munit_assert_int(strcmp(tc.text, "a\0"), ==, 0);
    munit_assert_char(tc.text[1], ==, '\0');

    SDLNW_TextControllerInsert(&tc, 'b', 0);
    munit_assert_int(strcmp(tc.text, "ba\0"), ==, 0);

    SDLNW_TextControllerInsert(&tc, 'c', 2);
    munit_assert_int(strcmp(tc.text, "bac\0"), ==, 0);

    SDLNW_DestroyTextController(&tc);

    return MUNIT_OK;
}

static MunitResult test_SDLNW_TextControllerRemove(const MunitParameter params[], void *user_data_or_fixture) {
    (void)params;
    (void)user_data_or_fixture;

    SDLNW_TextController tc;
    SDLNW_InitTextController(&tc);

    SDLNW_TextControllerInsert(&tc, 'a', 0);
    SDLNW_TextControllerInsert(&tc, 'b', 1);
    SDLNW_TextControllerInsert(&tc, 'c', 2);

    munit_assert_int(strcmp(tc.text, "abc\0"), ==, 0);

    SDLNW_TextControllerRemove(&tc, 10);
    munit_assert_int(strcmp(tc.text, "abc\0"), ==, 0);

    SDLNW_TextControllerRemove(&tc, 0);
    munit_assert_int(strcmp(tc.text, "bc\0"), ==, 0);
    SDLNW_TextControllerRemove(&tc, 1);
    munit_assert_int(strcmp(tc.text, "b\0"), ==, 0);
    SDLNW_TextControllerRemove(&tc, 0);
    munit_assert_int(strcmp(tc.text, "\0"), ==, 0);
    SDLNW_TextControllerRemove(&tc, 0);
    munit_assert_int(strcmp(tc.text, "\0"), ==, 0);
    SDLNW_TextControllerRemove(&tc, 0);
    munit_assert_int(strcmp(tc.text, "\0"), ==, 0);
    SDLNW_TextControllerRemove(&tc, 0);
    munit_assert_int(strcmp(tc.text, "\0"), ==, 0);

    SDLNW_DestroyTextController(&tc);

    return MUNIT_OK;
}

static void change_listener(void* data, char* str) {
    char* buffer = data;
    strcpy(buffer, str);
}

static MunitResult test_SDLNW_AddTextControllerChangeListener(const MunitParameter params[], void *user_data_or_fixture) {
    (void)params;
    (void)user_data_or_fixture;

    SDLNW_TextController tc;
    char buffer[30];

    SDLNW_InitTextController(&tc);

    SDLNW_AddTextControllerChangeListener(&tc, SDLNW_CreateTextControllerChangeListener(buffer, change_listener, NULL));

    // write some characters and remove some characters, check each time buffer is updated
    SDLNW_TextControllerInsert(&tc, 'a', 0);
    munit_assert_int(strcmp(buffer, "a\0"), ==, 0);
    SDLNW_TextControllerInsert(&tc, 'b', 0);
    munit_assert_int(strcmp(buffer, "ba\0"), ==, 0);
    SDLNW_TextControllerRemove(&tc, 0);
    munit_assert_int(strcmp(buffer, "a\0"), ==, 0);

    SDLNW_DestroyTextController(&tc);

    return MUNIT_OK;
}

static MunitResult test_SDLNW_SetTextControllerValue(const MunitParameter params[], void *user_data_or_fixture) {
    (void)params;
    (void)user_data_or_fixture;

    SDLNW_TextController tc;

    SDLNW_InitTextController(&tc);

    SDLNW_SetTextControllerValue(&tc, "Hello world!");
    munit_assert_int(strcmp(tc.text, "Hello world!"), ==, 0);

    SDLNW_SetTextControllerValue(&tc, "I am set with SDLNW_SetTextControllerValue!");
    munit_assert_int(strcmp(tc.text, "I am set with SDLNW_SetTextControllerValue!"), ==, 0);

    SDLNW_DestroyTextController(&tc);

    return MUNIT_OK;
}

// trivial but for completions sake
static MunitResult test_SDLNW_GetTextControllerValue(const MunitParameter params[], void *user_data_or_fixture) {
    (void)params;
    (void)user_data_or_fixture;

    SDLNW_TextController tc;

    SDLNW_InitTextController(&tc);

    munit_assert_ptr(SDLNW_GetTextControllerValue(&tc), ==, tc.text);

    SDLNW_DestroyTextController(&tc);

    return MUNIT_OK;
}

static MunitTest tests[] = {
    BUILD_TEST(test_SDLNW_InitTextController),
    BUILD_TEST(test_SDLNW_DestroyTextController),
    BUILD_TEST(test_SDLNW_TextControllerInsert),
    BUILD_TEST(test_SDLNW_TextControllerRemove),
    BUILD_TEST(test_SDLNW_AddTextControllerChangeListener),
    BUILD_TEST(test_SDLNW_SetTextControllerValue),
    BUILD_TEST(test_SDLNW_GetTextControllerValue),
    {0}
};

const MunitSuite text_controller_suite = {
    .prefix = "TextController ",
    .iterations = 1,
    .options = MUNIT_SUITE_OPTION_NONE,
    .suites = NULL,
    .tests = tests
};
