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

TODO

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
- [ ] Paragraph widget
- [ ] Text Field widget
- [ ] Text Entry widget
- [ ] Z stack widget
- [ ] Routing
- [x] widgets that can rebuild at runtime, composed of other widgets.
- [x] Build system
- [ ] Testing, inc renderering
- [ ] Mouse pointer updates, eg standard and clickable.

## Misc

- [ ] Create widget functions malloc multiple times, can be combined
- [ ] Prevent mistakes to have a generic widget initialiser
