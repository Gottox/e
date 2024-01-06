subdir('src')

subdir('include')

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
