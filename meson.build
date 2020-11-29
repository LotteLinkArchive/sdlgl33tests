project('sdlgl33tests', 'c')

cc = meson.get_compiler('c')

m_dep = cc.find_library('m', required : false)
sdl2_dep = dependency('sdl2')
gl_dep = dependency('GL')
thread_dep = dependency('threads')
glew_dep = dependency('glew')
cglm_dep = dependency('cglm')

gdeps = [m_dep, sdl2_dep, gl_dep, thread_dep, glew_dep]

executable('template', 'template.c', dependencies : gdeps)
executable('hello_triangle', 'hello_triangle.c', dependencies : gdeps)
executable('puck_square', 'puck_square.c', dependencies : gdeps)
executable('puck_spin', 'puck_spin.c', dependencies : [gdeps, cglm_dep])
executable('puck_cube', 'puck_cube.c', dependencies : [gdeps, cglm_dep])
