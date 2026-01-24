project(
    'e',
    'c',
    version: '0.1',
    default_options: [
        'warning_level=3',
        'quickjs:warning_level=2',
        'quickjs:werror=false',
        'termbox2:attr_width=64',
    ],
)

# system dependencies
cextras_dep = dependency('cextras')
tree_sitter_dep = dependency('tree-sitter')
termbox2_dep = dependency('termbox2')
quickjs_dep = dependency('quickjs-ng')

# project dependencies
liblsp_dep = subproject('liblsp').get_variable('liblsp_dep')
librope_dep = subproject('librope').get_variable('librope_dep')
libttyui_dep = subproject('libttyui').get_variable('libttyui_dep')
libeditorconfig_dep = subproject(
    'libeditorconfig',
).get_variable('libeditorconfig_dep')

libhighlight_dep = subproject('libhighlight').get_variable('libhighlight_dep')

#subdir('parsers')

subdir('include')

subdir('src')

dependencies = [
    quickjs_dep,
    cextras_dep,
    librope_dep,
    libttyui_dep,
    termbox2_dep,
    libeditorconfig_dep,
    #tree_sitter_dep,
]

executable(
    'e',
    e_src_main + e_src,
    include_directories: [
        e_include,
    ],
    dependencies: dependencies,
    install: true,
)

subdir('test')
