project('sdlgl33tests', 'c')

cc = meson.get_compiler('c')

m_dep = cc.find_library('m', required : false)
sdl2_dep = dependency('sdl2')
gl_dep = dependency('GL')
thread_dep = dependency('threads')

gdeps = [m_dep, sdl2_dep, gl_dep, thread_dep]

executable('template', 'template.c', dependencies : gdeps)
executable('testing', 'testing.c', dependencies : gdeps)
