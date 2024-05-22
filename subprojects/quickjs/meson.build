project('quickjs', 'c')

cmake = import('cmake')
quickjs_opts = cmake.subproject_options()
quickjs_opts.append_compile_args(
    'c',
    '-Wno-error',
    '-Wno-pedantic',
    '-Wno-all',
    '-Wno-extra',
)
quickjs_cmake = cmake.subproject('quickjs_cmake', options: quickjs_opts)
quickjs_dep = declare_dependency(
    # quickjs depends on -lm and -pthread, but cmake doesn't add it. So we add it here manually.
    dependencies: [
        quickjs_cmake.dependency('qjs'),
        meson.get_compiler('c').find_library('m', required: false),
        dependency('threads'),
    ],
)
