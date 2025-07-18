project('libhighlight', 'c')
subdir('src')

subdir('include')

cextras_dep = dependency('cextras')
tree_sitter_dep = dependency('tree-sitter')

libhighlight_dependencies = [cextras_dep, tree_sitter_dep]

c_args = []

libhighlight = static_library(
    'highlight',
    libhighlight_sources,
    include_directories: [
        libhighlight_include,
    ],

    install: false,
    c_args: c_args,
    dependencies: libhighlight_dependencies,
)

libhighlight_dep = declare_dependency(
    include_directories: [
        libhighlight_include,
    ],
    link_with: libhighlight,
    dependencies: [
        tree_sitter_dep,
    ],
)

subdir('tinker')

if get_option('test')
    subdir('test')
endif
