project('SDLNW', 'c',
default_options: ['warning_level=3'])

sdl2_dep = dependency('sdl2')
ttf_dep = dependency('SDL2_ttf')

# lib
sdlnw_inc = include_directories('src/lib/include')
sdlnw_lib = static_library('SDLNW',
'src/lib/src/button.c',
'src/lib/src/column.c', 
'src/lib/src/composite.c',
'src/lib/src/font.c',
'src/lib/src/internal_helpers.c', 
'src/lib/src/label.c', 
'src/lib/src/paragraph.c',
'src/lib/src/placeholder.c',
'src/lib/src/router.c',
'src/lib/src/SDLNW.c',
'src/lib/src/scroll.c',
'src/lib/src/sizedbox.c',
'src/lib/src/surface.c',
'src/lib/src/zstack.c',
include_directories: sdlnw_inc)

# unsure what this does but the subproject guide has it https://mesonbuild.com/Subprojects.html
# revisit
# install: true

sdlnw_dep = declare_dependency(include_directories: sdlnw_inc,
link_with: sdlnw_lib)

# example programs
executable('example_0_column', 'src/examples/0_column.c', dependencies: [sdl2_dep, ttf_dep, sdlnw_dep])
executable('example_1_composite_state', 'src/examples/1_composite_state.c', dependencies: [sdl2_dep, ttf_dep, sdlnw_dep])
executable('example_2_routing_and_scrolling', 'src/examples/2_routing_and_scrolling.c', dependencies: [sdl2_dep, ttf_dep, sdlnw_dep])
