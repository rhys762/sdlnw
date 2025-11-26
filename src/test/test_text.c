#include <munit.h>
#include <wchar.h>
#include "SDL_clipboard.h"
#include "SDL_keyboard.h"
#include "SDL_keycode.h"
#include "SDL_pixels.h"
#include "src/lib/include/SDLNW.h"
#include "test_misc.h"
#include "SDLNWInternal.h"

#define WHITE (SDL_Color) {.r = 0xFF, .g = 0xFF, .b = 0xFF, .a = 0xFF}
#define BLACK (SDL_Color) {.a = 0xFF}
#define LOREM "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua."

const SDL_Rect SIZE = {0, 0, 300, 300};

static MunitResult test_Text_draw(const MunitParameter params[], void *user_data_or_fixture) {
    (void)params;
    SDLNW_TextController tc;
    SDLNW_InitTextController(&tc);

    SDLNW_SetTextControllerValue(&tc, LOREM);

    SDLNW_Widget* w = SDLNW_CreateTextWidget((SDLNW_TextWidgetOptions) {
        .fg = BLACK,
        .font = user_data_or_fixture,
        .text_controller = &tc
    });
    SDLNW_SetWidgetBackground(w, SDLNW_CreateSolidBackground(WHITE));

    double diff = compareImages(RENDERED_DIR "test_Text_draw.png", TEST_DIR "test_Text_draw.png", w, &SIZE);
    munit_assert_double(diff, <=, 1.0);

    SDLNW_DestroyTextController(&tc);
    SDLNW_DestroyWidget(w);

    return MUNIT_OK;
}

static MunitResult test_Text_click(const MunitParameter params[], void *user_data_or_fixture) {
    (void)params;
    SDLNW_TextController tc;
    SDLNW_InitTextController(&tc);

    SDLNW_SetTextControllerValue(&tc, LOREM);

    SDLNW_Widget* w = SDLNW_CreateTextWidget((SDLNW_TextWidgetOptions) {
        .fg = BLACK,
        .font = user_data_or_fixture,
        .text_controller = &tc,
        .selectable = true,
        .editable = true
    });
    SDLNW_SetWidgetBackground(w, SDLNW_CreateSolidBackground(WHITE));
    SDLNW_SetWidgetNetSize(w, &SIZE);

    dummy_render(w);

    // set the cursor in the middle
    SDLNW_ClickWidget(w, 150, 150, 1);

    SDLNW_TextInputEvent tinput = {
        .text = "hello_world"
    };

    SDLNW_TrickleDownEvent(w, SDLNW_EventType_TextInput, &tinput, NULL);
    dummy_render(w);

    const char* text = SDLNW_GetTextControllerValue(&tc);

    munit_assert_string_not_equal(text, LOREM);
    munit_assert_string_equal(text, "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod temphello_worldor incididunt ut labore et dolore magna aliqua.");

    // check double click
    SDLNW_ClickWidget(w, 83, 178, 2); // the word we inserted hello_world into, measured with GIMP

    SDLNW_KeyUpEvent keyup = {
        .key = SDLK_BACKSPACE
    };

    // delete the selected section
    SDLNW_TrickleDownEvent(w, SDLNW_EventType_KeyUp, &keyup, NULL);
    dummy_render(w);

    text = SDLNW_GetTextControllerValue(&tc);

    munit_assert_string_equal(text, "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod  incididunt ut labore et dolore magna aliqua.");

    SDLNW_DestroyTextController(&tc);
    SDLNW_DestroyWidget(w);

    return MUNIT_OK;
}

static MunitResult test_Text_drag(const MunitParameter params[], void *user_data_or_fixture) {
    (void)params;
    SDLNW_TextController tc;
    SDLNW_InitTextController(&tc);

    SDLNW_SetTextControllerValue(&tc, LOREM);

    SDLNW_Widget* w = SDLNW_CreateTextWidget((SDLNW_TextWidgetOptions) {
        .fg = BLACK,
        .font = user_data_or_fixture,
        .text_controller = &tc,
        .highlight = (SDL_Colour) {.r = 0xFF, .a = 0x66},
        .selectable = true
    });

    SDLNW_SetWidgetNetSize(w, &SIZE);
    dummy_render(w);
    __sdlnw_clear_text_widget_cache(w);

    SDLNW_DragWidget(w, 40, 80, 40, 80, true);
    SDLNW_DragWidget(w, 40, 80, 101, 109, false); // gimp TODO fix this, currently needs drag in two steps

    SDLNW_SetWidgetBackground(w, SDLNW_CreateSolidBackground(WHITE));

    double diff = compareImages(RENDERED_DIR "test_Text_drag.png", TEST_DIR "test_Text_drag.png", w, NULL);
    munit_assert_double(diff, <=, 1.0);

    SDLNW_DestroyTextController(&tc);
    SDLNW_DestroyWidget(w);

    return MUNIT_OK;
}

static MunitResult test_Text_get_size(const MunitParameter params[], void *user_data_or_fixture) {
    (void)params;

    SDLNW_TextController tc;
    SDLNW_InitTextController(&tc);

    SDLNW_SetTextControllerValue(&tc, LOREM);

    SDLNW_Widget* w = SDLNW_CreateTextWidget((SDLNW_TextWidgetOptions) {
        .fg = BLACK,
        .font = user_data_or_fixture,
        .text_controller = &tc
    });

    SDLNW_SizeResponse resp = SDLNW_GetWidgetRequestedSize(w, (SDLNW_SizeRequest) {
        .total_pixels_avaliable_width = 200
    });

    munit_assert_int(resp.height.pixels, >=, 250);
    munit_assert_int(resp.height.pixels, <=, 300);

    resp = SDLNW_GetWidgetRequestedSize(w, (SDLNW_SizeRequest) {
        .total_pixels_avaliable_height = 200
    });
    munit_assert_int(resp.width.pixels, >=, 250);
    munit_assert_int(resp.width.pixels, <=, 300);

    // zero avaliable pixels, single line
    resp = SDLNW_GetWidgetRequestedSize(w, (SDLNW_SizeRequest) {0});
    munit_assert_int(resp.width.pixels, >=, 1400);
    munit_assert_int(resp.height.pixels, >=, 20);

    SDLNW_DestroyTextController(&tc);
    SDLNW_DestroyWidget(w);

    return MUNIT_OK;
}

static MunitResult test_Text_copy_paste(const MunitParameter params[], void *user_data_or_fixture) {
    (void)params;

    SDLNW_TextController tc;
    SDLNW_InitTextController(&tc);

    SDLNW_SetTextControllerValue(&tc, LOREM);

    SDLNW_Widget* w = SDLNW_CreateTextWidget((SDLNW_TextWidgetOptions) {
        .fg = BLACK,
        .font = user_data_or_fixture,
        .text_controller = &tc,
        .selectable = true,
        .editable = true
    });

    SDLNW_SetWidgetNetSize(w, &SIZE);

    dummy_render(w);

    SDLNW_DragWidget(w, 40, 80, 40, 80, true);
    SDLNW_DragWidget(w, 40, 80, 101, 109, false); // gimp TODO fix this, currently needs drag in two steps

    SDL_SetModState(KMOD_CTRL);

    SDLNW_KeyUpEvent evt = {
        .key = SDLK_c
    };

    // should copy to clipboard
    SDLNW_TrickleDownEvent(w, SDLNW_EventType_KeyUp, &evt, NULL);

    munit_assert_string_equal(SDL_GetClipboardText(), "em ipsum dolor sit amet, c");

    // try pasting
    SDL_SetClipboardText("I like apples");
    evt.key = SDLK_v;
    SDLNW_TrickleDownEvent(w, SDLNW_EventType_KeyUp, &evt, NULL);
    const char* text = SDLNW_GetTextControllerValue(&tc);
    munit_assert_string_equal(text, "LorI like applesonsectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.");

    SDL_SetModState(KMOD_NONE);
    SDLNW_DestroyTextController(&tc);
    SDLNW_DestroyWidget(w);

    return MUNIT_OK;
}

static MunitTest tests[] = {
    BUILD_TEST(test_Text_draw),
    BUILD_TEST(test_Text_click),
    BUILD_TEST(test_Text_drag),
    BUILD_TEST(test_Text_get_size),
    BUILD_TEST(test_Text_copy_paste),
    {0}
};

const MunitSuite text_suite = {
    .prefix = "Text ",
    .iterations = 1,
    .options = MUNIT_SUITE_OPTION_NONE,
    .suites = NULL,
    .tests = tests
};
