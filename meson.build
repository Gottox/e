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

cextras_dep = dependency('cextras')
tree_sitter_dep = dependency('tree-sitter')
#tree_sitter_dep = subproject('tree-sitter').get_variable('tree_sitter_dep')

quickjs_dep = subproject('quickjs').get_variable('quickjs_dep')

liblsp_dep = subproject('liblsp').get_variable('liblsp_dep')

librope_dep = subproject('librope').get_variable('librope_dep')

libttyui_dep = subproject('libttyui').get_variable('libttyui_dep')

subdir('parsers')

libhighlight_dep = subproject('libhighlight').get_variable('libhighlight_dep')

subdir('include')

subdir('src')

dependencies = [
    quickjs_dep,
    cextras_dep,
    librope_dep,
    libttyui_dep,
    #tree_sitter_dep,
]

executable(
    'e',
    e_src,
    include_directories: [
        e_include,
    ],
    dependencies: dependencies,
    install: true,
)

subdir('test')
