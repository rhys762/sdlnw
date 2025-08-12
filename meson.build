project('SDLNW', 'c',
default_options: ['warning_level=3'])

# sdl2
sdl2_dep = dependency('sdl2')
ttf_dep = dependency('SDL2_ttf')

# munit
munit_proj = subproject('munit')
munit_dep = munit_proj.get_variable('munit_dep')

# lib
sdlnw_inc = include_directories('src/lib/include')
sdlnw_lib = static_library('SDLNW',
'src/lib/src/canvas.c', 
'src/lib/src/centre.c', 
'src/lib/src/column.c', 
'src/lib/src/composite.c',
'src/lib/src/font.c',
'src/lib/src/gesture.c',
'src/lib/src/internal_helpers.c', 
'src/lib/src/label.c', 
'src/lib/src/placeholder.c',
'src/lib/src/router.c',
'src/lib/src/SDLNW.c',
'src/lib/src/scroll.c',
'src/lib/src/sizedbox.c',
'src/lib/src/surface.c',
'src/lib/src/zstack.c',
'src/lib/src/text_controller.c',
'src/lib/src/text.c',
'src/lib/src/text_change_listener.c',
'src/lib/src/rendered_text.c',
include_directories: sdlnw_inc)

# unsure what this does but the subproject guide has it https://mesonbuild.com/Subprojects.html
# revisit
# install: true

cc = meson.get_compiler('c')

sdlnw_dep = declare_dependency(include_directories: sdlnw_inc,
link_with: sdlnw_lib)

# std math lib
m_dep = cc.find_library('m', required : false)

# example programs
executable('example_0_column', 'src/examples/0_column.c', dependencies: [sdl2_dep, ttf_dep, sdlnw_dep])
executable('example_1_composite_state', 'src/examples/1_composite_state.c', dependencies: [sdl2_dep, ttf_dep, sdlnw_dep])
executable('example_2_routing_and_scrolling', 'src/examples/2_routing_and_scrolling.c', dependencies: [sdl2_dep, ttf_dep, sdlnw_dep])
executable('example_3_canvas', 'src/examples/3_canvas.c', dependencies: [sdl2_dep, ttf_dep, sdlnw_dep, m_dep])
executable('example_4_form', 'src/examples/4_form.c', dependencies: [sdl2_dep, ttf_dep, sdlnw_dep, m_dep])

# tests
if not meson.is_subproject()
    executable('run_tests',
    'src/test/main.c',
    'src/test/test_text_controller.c',
    dependencies: [sdl2_dep, sdlnw_dep, munit_dep])
endif
