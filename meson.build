project(
    'e',
    'c',
    version: '0.1',
    default_options: [
        'warning_level=3',
        'quickjs:warning_level=2',
        'quickjs:werror=false',
        'quickjs:libc=false',
        'quickjs:default_library=static',
    ],
)

quickjs_dep = dependency('quickjs')
cextras_dep = dependency('cextras')
tree_sitter_dep = dependency('tree-sitter')
#tree_sitter = subproject('tree-sitter').get_variable('tree_sitter_dep')

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
    #tree_sitter,
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
