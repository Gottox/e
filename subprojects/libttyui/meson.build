project('libttyui', 'c')
subdir('src')

subdir('include')

cextras_dep = dependency('cextras')

libttyui_dependencies = [cextras_dep]

c_args = []

libttyui = static_library(
    'ttyui',
    libttyui_sources,
    include_directories: [
        libttyui_include,
    ],

    install: false,
    c_args: c_args,
    dependencies: libttyui_dependencies,
)

libttyui_dep = declare_dependency(
    include_directories: [
        libttyui_include,
    ],
    link_with: libttyui,
)
