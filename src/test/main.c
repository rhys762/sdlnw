#include "SDL.h"
#include "SDLNW.h"
#include "src/test/test_misc.h"
#include <stdio.h>
#include <munit.h>
#include <libgen.h>
#include "sys/stat.h"
#include "SDL_image.h"
#include <assert.h>

extern const MunitSuite placeholder_suite;
extern const MunitSuite column_suite;
extern const MunitSuite text_controller_suite;
extern const MunitSuite composite_suite;
extern const MunitSuite text_suite;
extern const MunitSuite zstack_suite;
extern const MunitSuite border_suite;

int main(int argc, char** argv) {
    SDL_Init(SDL_INIT_EVERYTHING);
    TTF_Init();
    IMG_Init(IMG_INIT_PNG);

    MunitSuite suites[] = {
        column_suite,
        composite_suite,
        placeholder_suite,
        text_controller_suite,
        text_suite,
        zstack_suite,
        border_suite,
        {0}
    };

    MunitSuite suite = {
        .prefix = "SDLNW ",
        .suites = suites
    };

    // clean up last test
    remove(RENDERED_DIR);
    mkdir(RENDERED_DIR, 0777);

    // pass font as user_data
    const char* dir = dirname(argv[0]);
    char* fontpath = NULL;
    SDL_asprintf(&fontpath, "%s/../jbm/fonts/ttf/JetBrainsMonoNL-Regular.ttf", dir);
    SDLNW_Font* font = SDLNW_CreateFont(fontpath, 20);
    assert(font != NULL);

    int val = munit_suite_main(&suite, font, argc, argv);
    SDLNW_DestroyFont(font);
    free(fontpath);
    SDLNW_ReportLeaks();

    IMG_Quit();
    TTF_Quit();
    SDL_Quit();

    return val;
}
