#include "SDLNWInternal.h"
#include <SDL2/SDL_blendmode.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_ttf.h>
#include <assert.h>

static void  destroy_rendered_char(__sdlnw_RenderedChar* c) {
    if (c->surface) {
        SDL_FreeSurface(c->surface);
    }
}

static void clear_chars(__sdlnw_RenderedText* rt) {
    for (size_t i = 0; i < rt->chars_len; i++) {
        destroy_rendered_char(&rt->chars[i]);
    }

    rt->chars_len = 0;
}

void __sdlnw_RenderedText_init(__sdlnw_RenderedText* rt, SDLNW_Font* font, SDL_Colour c) {
    *rt = (__sdlnw_RenderedText) {
        .font = font
    };

    rt->chars_cap = 10;
    rt->chars = __sdlnw_malloc(rt->chars_cap * sizeof(__sdlnw_RenderedChar));
    rt->colour = c;
}

void __sdlnw_RenderedText_destroy(__sdlnw_RenderedText* rt) {
    __sdlnw_free(rt->chars);
    *rt = (__sdlnw_RenderedText){0};
}

// something that represents a break in a word
static bool is_break(char c) {
    return c == ' ' || c == '.' || c == ',' || c == '?';
}

void __sdlnw_RenderedText_set_text(__sdlnw_RenderedText* rt, const char* text, int avaliable_width) {
    // clear existing
    clear_chars(rt);

    SDL_DestroyTexture(rt->texture);
    rt->texture = NULL;

    rt->width = avaliable_width;

    size_t len = strlen(text);

    if (!len) {
        return;
    }

    // ensure enough space is allocated in buffer
    if (len >= rt->chars_cap) {
        while (len >= rt->chars_cap) {
            rt->chars_cap *= 2;
        }

        rt->chars = __sdlnw_realloc(rt->chars, rt->chars_cap * sizeof(__sdlnw_RenderedChar));
        assert(rt->chars != NULL);
    }

    char str[2] = {
        '\0',
        '\0'
    };

    SDL_Rect dest = {0};

    for (size_t i = 0; i < len; i++) {
        char c = text[i];
        str[0] = c;

        if (c == '\n') {
            dest.x = 0;
            dest.y += rt->font->line_height;
            dest.w = 0;
            dest.h = 0;

            rt->chars[i] = (__sdlnw_RenderedChar) {
                .c = c,
                .rendered_to = dest
            };

            continue;
        }

        SDL_Surface* surface = TTF_RenderText_Blended(rt->font->font, str, rt->colour);

        dest.w = surface->w;
        dest.h = surface->h;

        rt->chars[i] = (__sdlnw_RenderedChar) {
            .c = c,
            .surface = surface,
            .native_width = surface->w,
            .native_height = surface->h,
            .rendered_to = dest
        };

        if ((dest.x + dest.w) > avaliable_width) {
            // over ran width, backtrack until start of word
            size_t j = i;
            while (j > 0 && !is_break(text[j - 1])) {
                j--;
            }

            if (!rt->chars[j].rendered_to.x) {
                // was line break anyway
                continue;
            }

            dest.x = 0;
            dest.y += rt->font->line_height;
            for (; j <= i; j++) {
                dest.w = rt->chars[j].native_width;
                dest.h = rt->chars[j].native_height;
                rt->chars[j].rendered_to = dest;
                dest.x += dest.w;
            }
        }

        dest.x += dest.w;
    }

    SDL_Rect last = rt->chars[len - 1].rendered_to;
    rt->height = last.y + last.h;
    rt->chars_len = len;
}

void __sdlnw_RenderedText_render_text(__sdlnw_RenderedText* rt, SDL_Renderer* renderer, int x, int y) {
    if (!rt->chars_len) {
        return;
    }

    if (!rt->texture) {
        rt->texture = NULL;
        SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, rt->width, rt->height, 32, SDL_PIXELFORMAT_RGBA8888);
        SDL_FillRect(surface, NULL, SDL_MapRGBA(surface->format, 0, 0, 0, 0));

        SDL_Rect dst = {0};
        for (size_t i = 0; i < rt->chars_len; i++) {
            __sdlnw_RenderedChar* c = &rt->chars[i];

            dst = (SDL_Rect){
                .x = c->rendered_to.x,
                .y = c->rendered_to.y,
                .w = c->rendered_to.w,
                .h = c->rendered_to.h,
            };

            SDL_BlitSurface(rt->chars[i].surface, NULL, surface, &dst);
        }

        rt->texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
    }

    SDL_Rect dst = {
        .x = x,
        .y = y,
        .w = rt->width,
        .h = rt->height
    };
    SDL_RenderCopy(renderer, rt->texture, NULL, &dst);
}

void __sdlnw_RenderedText_render_cursor(__sdlnw_RenderedText* rt, SDL_Renderer* renderer, size_t cursor, const SDL_Colour* colour, int xorigin, int yorigin) {
    // find x and y of cursor
    int x = 0, y = 0;

    int h = rt->font->line_height;

    if (cursor > 0) {
        __sdlnw_RenderedChar* c = &rt->chars[cursor - 1];
        x = c->rendered_to.x + c->rendered_to.w;
        y = c->rendered_to.y;
    }

    SDL_SetRenderDrawColor(renderer, colour->r, colour->g, colour->b, colour->a);
    SDL_RenderDrawLine(renderer, xorigin + x, yorigin + y, xorigin + x, yorigin + y + h);
}

void __sdlnw_RenderedText_render_highlight(__sdlnw_RenderedText* rt, SDL_Renderer* renderer, size_t from, size_t to, const SDL_Colour* colour, int xorigin, int yorigin) {
    SDL_SetRenderDrawColor(renderer, colour->r, colour->g, colour->b, colour->a);

    size_t low = from;
    size_t high = to;

    if (low > high) {
        high = from;
        low = to;
    }

    SDL_Rect dst = {0};

    for (size_t i = low; i < high; i++) {
        dst = rt->chars[i].rendered_to;
        dst.x += xorigin;
        dst.y += yorigin;
        SDL_RenderFillRect(renderer, &dst);
    }
}

size_t __sdlnw_RenderedText_convert_point_to_char_index(__sdlnw_RenderedText* rt, int x, int y) {
    // TODO y
    (void)y;
    size_t index = (size_t)-1;

    for (size_t i = 0; i < rt->chars_len; i++) {
        __sdlnw_RenderedChar* c = &rt->chars[i];

        if (x < (c->rendered_to.x + c->rendered_to.w/2) &&
            y >= c->rendered_to.y &&
            y < c->rendered_to.y + c->rendered_to.h) {

            index = i;
            break;
        }
    }

    return index;
}
