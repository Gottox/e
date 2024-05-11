project(
    'e',
    'c',
    version: '0.1',
    default_options: [
        'warning_level=3',
        'quickjs:warning_level=2',
        'quickjs:werror=false',
    ],
)

# Configure QuickJS
cmake = import('cmake')
quickjs = cmake.subproject('quickjs')

# Dependencies
quickjs_dep = declare_dependency(
    # quickjs depends on -lm and -pthread, but cmake doesn't add it. So we add it here manually.
    dependencies: [
        quickjs.dependency('qjs'),
        meson.get_compiler('c').find_library('m', required: false),
        dependency('threads'),
    ],
)
cextras_dep = dependency('cextras')
tree_sitter_dep = dependency('tree-sitter')
#tree_sitter_dep = subproject('tree-sitter').get_variable('tree_sitter_dep')

subdir('librope')

subdir('libttyui')

subdir('parsers')

subdir('libhighlight')

subdir('include')

subdir('src')

dependencies = [
    quickjs_dep,
    cextras_dep,
    librope_dep,
    libttyui_dep,
    #tree_sitter_dep,
]

exe = executable(
    'e',
    e_src,
    include_directories: [
        e_include,
    ],
    dependencies: dependencies,
    install: true,
)

subdir('test')
