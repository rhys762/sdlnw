
> [!WARNING]
> This library is in early development and is both unstable and incomplete.

# About

SDL Nested Widgets is intended to be a small library for managing simple ui elements (like buttons) using the SDL libraries.

# Requirements

- SDL2, SDL2_ttf development libraries.
- meson (and toolchain, see https://mesonbuild.com/Quick-guide.html)
- typical linux tools, including but not limited to: make, wget, unzip, a c compiler

# Building

## First time setup

Download Jet Brains Mono:

```bash
make font
ls jbm/fonts/ttf/JetBrainsMono-Regular.ttf # success
```

Create the build directory:

```bash
make setup
```

## Compiling

```bash
make build
```

# Tests and Examples

Tests are still todo. What little exist can be run with:

```bash
make test
```

Test coverage can be produced with
```bash
make coverage
```

Example code is in `src/examples` and are built in `build`. Run with:

```bash
build/example_0_todo
```

# Resource Allocation

TODO, fill out this more when the API settles down.

Widget's take ownership of any child widgets, typically you will only need to destroy the root of your widget hierarchy:

```c
SDLNW_Widget* p = SDLNW_CreatePlaceholderWidget();
SDLNW_Widget* scroll = SDLNW_CreateScrollWidget(p); // scroll now owns p
```

Widget's that accept a null terminated array of child widgets take control of those widgets, but NOT the array itself:

```c
SDLNW_Widget* widgets[] = {
    widget_a,
    widget_b,
    NULL
};

SDLNW_Widget* column = SDLNW_CreateColumnWidget(widgets); // column will destroy widget_a and widget_b, but will NOT try to free widgets
```
So you need to handle deallocation of heap arrays:
```c
SDLNW_Widget** widgets = malloc(sizeof(SDLNW_Widget*) * 10);
widgets[0] = widget_a;
widgets[1] = widget_b;
widgets[2] = NULL;

SDLNW_Widget* column = SDLNW_CreateColumnWidget(widgets); // column takes ownership of widget_a and widget_b, but NOT widgets
free(widgets);
```

Widgets that accept pointers to some kind of state or otherwise do NOT take ownership of that data by default:

```c
struct some_struct s = {0};
SDLNW_Widget* button = SDLNW_CreateGestureDetectorWidget(child_widget, (SDLNW_GestureDetectorWidget_Options){.data=&s, .on_click=on_click});
// OK, button has a pointer to a local variable, but will not try to free
```

However, in the case that you do want such a pointer cleaned up, widgets can be told to do so at the end of their lifetime:

```c
struct some_struct* s = malloc(sizeof(struct some_struct));
*s = (struct some_struct){0};
SDLNW_Widget* button = SDLNW_CreateGestureDetectorWidget(child_widget, (SDLNW_GestureDetectorWidget_Options){.data=&s, .on_click=on_click});
// button should free s when it is destroyed:
SDLNW_Widget_AddOnDestroy(button, s, free);
```

# Integration

## With a Meson Project

Add SDLNW as a git submodule (meson requires the `subprojects` directory):

```bash
git submodule add git@github.com:rhys762/sdlnw.git subprojects/sdlnw
```

Then in your projects meson.build:

```meson
# sdl and ttf as usual
sdl2_dep = dependency('sdl2')
ttf_dep = dependency('SDL2_ttf')

# sdlnw
sdlnw_proj = subproject('sdlnw')
sdlnw_dep = sdlnw_proj.get_variable('sdlnw_dep')

# your program build step
executable('my_program', 'main.c', dependencies: [sdl2_dep, ttf_dep, sdlnw_dep]) # TODO add SDLNW deps, we atleast need maths
```

A minimal sanity check main.c would be:

```c
// TODO
```

Once you've built once, your language server will probably be able to find all the declaration's. It worked out of the box for me with clangd.

## Otherwise

Add SDLNW as a git submodule:

```bash
git submodule add git@github.com:rhys762/sdlnw.git subprojects/sdlnw
```

Your consuming application will need to include `subprojects/sdlnw/src/lib/include/SDLNW.h`.

Then:
```bash
cd subprojects/sdlnw
meson setup build
cd build
ninja
```

Which should produce `subprojects/sdlnw/build/libSDLNW.a`, which you can then link against.

# TODO List

## Features

In no particular order.

- [x] Placeholder widget
- [x] Row widget
- [x] Column widget
- [x] Surface widget
- [x] Gesture widget
- [ ] Styling
- [x] Label widget
- [x] Text widget
- [x] Z stack widget
- [x] Routing
- [x] widgets that can rebuild at runtime, composed of other widgets.
- [x] Build system
- [ ] Testing, inc renderering
- [x] Mouse pointer updates, eg standard and clickable.
- [x] Scroll widget
- [ ] Animation widget
- [ ] Checkbox
- [ ] Slider
- [x] Sized box
- [x] Center box
- [ ] Dropdown
- [x] Canvas

## Misc

- Create widget functions malloc multiple times, can be combined
- Const correctness
- Font render caching
- The composite recompose should assert that its actually got a composite widget, instead of being UB
- how much of sdlnw.h can be moved to internal helpers
