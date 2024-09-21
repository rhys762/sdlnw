# About

SDL Nested Widgets is intended to be a small library for managing simple ui elements (like buttons) using the SDL libraries.

# Requirements

- SDL2 development libraries.
- meson (and toolchain, see https://mesonbuild.com/Quick-guide.html)

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

In no particular order.

- [x] Placeholder widget
- [ ] Row widget
- [x] Column widget
- [x] Surface widget
- [x] Button widget
- [ ] Styling
- [ ] Label widget
- [ ] Paragraph widget
- [ ] Text Field widget
- [ ] Text Entry widget
- [ ] Z stack widget
- [ ] Routing
- [x] widgets that can rebuild at runtime, composed of other widgets.
- [x] Build system
- [ ] Testing, inc renderering