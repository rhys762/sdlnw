# About

SDL Nested Widgets is intended to be a small library for managing simple ui elements (like buttons) using the SDL libraries.

# Requirements

- SDL2, SDL2_ttf development libraries.
- meson (and toolchain, see https://mesonbuild.com/Quick-guide.html)
- typical linux tools, including but not limited to: wget, unzip

# Building

```bash
meson setup build
cd build
ninja
```

# Tests and Examples

Tests are still todo.

Example code is in src/examples. 

# Resource Allocation

TODO

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
- [ ] Scroll widget
- [ ] Animation widget
- [ ] Checkbox (maybe)
- [ ] Slider (maybe)

## Misc

- Create widget functions malloc multiple times, can be combined
- Const correctness
- Font render caching
- The composite recompose should assert that its actually got a composite widget, instead of being UB
- Text selection
- Clipboard
