#ifndef TEST_MISC_H
#define TEST_MISC_H

#include "SDLNW.h"

#define RENDERED_DIR "test_renders/"
#define TEST_DIR "../render_test_cases/"

#define BUILD_TEST(FUNC) { .name = #FUNC, .test = FUNC}

// fuzzy compare two images and return a diff value thats been averaged over pixels, acceptable diffs will differ from test to test
double compareImages(const char* save_as, const char* test_against, SDLNW_Widget* widget, const SDL_Rect* size);
// sdl2 does not provide an option for this
SDL_Surface* createSurfaceFromTexture(SDL_Renderer* renderer, SDL_Texture* texture);
// composite resolves buffers on render (since you can guarentee its not pumping events)
void dummy_render(SDLNW_Widget* w);

#endif
