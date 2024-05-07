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

dependencies = [
    quickjs_dep,
    cextras_dep,
    #tree_sitter,
]

subdir('librope')

subdir('libttyui')

subdir('libhighlight')

subdir('parsers')

subdir('include')

subdir('src')

exe = executable(
    'e',
    e_src,
    include_directories: [
        e_include,
        librope_include,
        libttyui_include,
    ],
    link_with: [
        librope,
        libttyui,
    ],
    dependencies: dependencies,
    install: true,
)

subdir('test')
