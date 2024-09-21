project('SDLNW', 'c')

sdl2_dep = dependency('sdl2')

# lib
sdlnw_lib = static_library('SDLNW',
'src/lib/button.c',
'src/lib/column.c', 
'src/lib/composite.c',
'src/lib/internal_helpers.c', 
'src/lib/placeholder.c',
'src/lib/SDLNW.c',
'src/lib/surface.c')

# example programs
executable('example_0_column', 'src/examples/0_column.c', dependencies: [sdl2_dep], link_with: [sdlnw_lib])
executable('example_1_composite_state', 'src/examples/1_composite_state.c', dependencies: [sdl2_dep], link_with: [sdlnw_lib])