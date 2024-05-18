project('quickjs', 'c')

cmake = import('cmake')
quickjs = cmake.subproject('quickjs_cmake')

quickjs_dep = declare_dependency(
    # quickjs depends on -lm and -pthread, but cmake doesn't add it. So we add it here manually.
    dependencies: [
        quickjs.dependency('qjs'),
        meson.get_compiler('c').find_library('m', required: false),
        dependency('threads'),
    ],
)
