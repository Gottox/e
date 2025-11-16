e_test = [
    'ttyui.c',
    'ttyui_events.c',
    'ttyui_draw.c',
]

testlib_dep = dependency('testlib')
cc = meson.get_compiler('c')
libutil_dep = cc.find_library('util', required: false)

dependencies = [
    testlib_dep,
    libttyui_dep,
]

if libutil_dep.found()
    dependencies += libutil_dep
endif

foreach p : e_test
    sources = [p]
    t = executable(
        p.underscorify(),
        sources,
        install: false,
        dependencies: dependencies,
    )
    test(p, t)
endforeach
