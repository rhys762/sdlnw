project('SDLNW', 'c', default_options: ['warning_level=3', 'c_std=c17'])

if not meson.is_subproject()
  add_project_arguments('-pg', language: 'c')
  add_project_link_arguments('-pg', language: 'c')
endif

# sdl2
sdl2_dep = dependency('sdl2')
ttf_dep = dependency('SDL2_ttf')
image_dep = dependency('SDL2_image')

# munit
munit_proj = subproject('munit')
munit_dep = munit_proj.get_variable('munit_dep')

# lib files
lib_src = files(
  'src/lib/src/SDLNW.c',
  'src/lib/src/background.c',
  'src/lib/src/border.c',
  'src/lib/src/canvas.c',
  'src/lib/src/centre.c',
  'src/lib/src/column.c',
  'src/lib/src/composite.c',
  'src/lib/src/font.c',
  'src/lib/src/gesture.c',
  'src/lib/src/internal_helpers.c',
  'src/lib/src/placeholder.c',
  'src/lib/src/prefab/button.c',
  'src/lib/src/prefab/label.c',
  'src/lib/src/rendered_text.c',
  'src/lib/src/router.c',
  'src/lib/src/row.c',
  'src/lib/src/scroll.c',
  'src/lib/src/sizedbox.c',
  'src/lib/src/spline.c',
  'src/lib/src/text.c',
  'src/lib/src/text_change_listener.c',
  'src/lib/src/text_controller.c',
  'src/lib/src/widget_set.c',
  'src/lib/src/zstack.c',
)

# lib
sdlnw_inc = include_directories('src/lib/include')

sdlnw_lib = static_library(
  'SDLNW',
  lib_src,
  include_directories: sdlnw_inc,
  c_args: ['-O3']
)

sdlnw_debug_lib = static_library(
  'SDLNW_debug',
  lib_src,
  include_directories: sdlnw_inc,
  c_args: ['-DSDLNW_DEBUG=1', '-g', '-O0']
)


cc = meson.get_compiler('c')

# std math lib
m_dep = cc.find_library('m', required: false)

sdlnw_dep = declare_dependency(include_directories: sdlnw_inc, link_with: sdlnw_lib, dependencies: [sdl2_dep, ttf_dep, m_dep])
sdlnw_debug_dep = declare_dependency(include_directories: sdlnw_inc, link_with: sdlnw_debug_lib, dependencies: [sdl2_dep, ttf_dep, m_dep],)

# tests
if not meson.is_subproject()
  # example programs
  executable(
    'example_0_todo',
    'src/examples/example_0_todo/main.c',
    dependencies: [sdlnw_debug_dep],
  )

  e = executable(
    'run_tests',
    'src/test/main.c',
    'src/test/test_misc.c',
    'src/test/test_text_controller.c',
    'src/test/test_placeholder.c',
    'src/test/test_column.c',
    'src/test/test_composite.c',
    'src/test/test_text.c',
    'src/test/test_zstack.c',
    'src/test/test_border.c',
    lib_src,
    dependencies: [sdlnw_debug_dep, munit_dep, image_dep],
  )

  test('Unit Tests', e)
endif
