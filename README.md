
> [!WARNING]  
> This library is early development and is both unstable and incomplete.

# About

SDL Nested Widgets is intended to be a small library for managing simple ui elements (like buttons) using the SDL libraries.

# Requirements

- SDL2, SDL2_ttf development libraries.
- meson (and toolchain, see https://mesonbuild.com/Quick-guide.html)
- typical linux tools, including but not limited to: wget, unzip

# Building

## First time setup

Download Jet Brains Mono:

```bash
./scripts/get_jbm.bash
ls jbm/fonts/ttf/JetBrainsMono-Regular.ttf # success
```

Create the build directory and build:

```bash
meson setup build
cd build
ninja
```

## Following builds

```bash
cd build
ninja
```

# Tests and Examples

Tests are still todo.

Example code is in `src/examples` and are built in `build`. Run with:

```bash
build/example_2_routing_and_scrolling
```

Or, with valgrind:

```bash
valgrind --leak-check=full --track-origins=yes --show-leak-kinds=all --gen-suppressions=all --suppressions=./suppress.valgrind build/example_2_routing_and_scrolling &> v.log
```

Some of the suppression rules may need adjusting, probably need to revisit the wildcards for leaks originating in main.

Examples start with a number that denotes intended reading order, and increase in complexity.

# Resource Allocation

Widget's and WidgetList's take ownership of any child widgets, typically you will only need to destroy the root of your widget hierarchy:

```c
SDLNW_Widget* p = SDLNW_CreatePlaceholderWidget();
SDLNW_Widget* button = SDLNW_CreateButtonWidget(p, NULL, cb);

// run app
const SDLNW_BootstrapOptions options = (SDLNW_BootstrapOptions){0};
SDLNW_bootstrap(button, options);    

// finished running
// button has ownership of p, only need to destroy button
SDLNW_Widget_Destroy(button);
```

In contrast, widgets that accept pointers to some kind of state or otherwise do NOT take ownership of that data by default:

```c
struct some_struct s = {0};
SDLNW_Widget* button = SDLNW_CreateButtonWidget(p, &s, cb);
// OK, button will not try to free s.
```

However, in the case that you do want such a pointer cleaned up, widgets can be told to do so at the end of their lifetime:

```c
struct some_struct* s = malloc(sizeof(struct some_struct));
*s = (struct some_struct){0};
SDLNW_Widget* button = SDLNW_CreateButtonWidget(p, s, cb);
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
executable('my_program', 'main.c', dependencies: [sdl2_dep, ttf_dep, sdlnw_dep])
```

Once you've built once, your language server will probably be able to find all the declaration's. It worked out of the box for me with vs-code and clangd.

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
- [ ] Row widget
- [x] Column widget
- [x] Surface widget
- [x] Button widget
- [ ] Styling
- [x] Label widget
- [x] Paragraph widget
- [ ] Text Field widget
- [ ] Text Entry widget
- [x] Z stack widget
- [x] Routing
- [x] widgets that can rebuild at runtime, composed of other widgets.
- [x] Build system
- [ ] Testing, inc renderering
- [x] Mouse pointer updates, eg standard and clickable.
- [x] Scroll widget
- [ ] Animation widget
- [ ] Checkbox (maybe)
- [ ] Slider (maybe)
- [x] Sized box
- [ ] Center box

## Misc

- Create widget functions malloc multiple times, can be combined
- Const correctness
- Font render caching
- The composite recompose should assert that its actually got a composite widget, instead of being UB
- Text selection
- Clipboard
