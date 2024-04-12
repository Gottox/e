subdir('src')

subdir('include')

libhightlight_dependencies = [cextras_dep, tree_sitter_dep]

c_args = []

libhighlight = static_library(
    'hightlight',
    libhighlight_sources,
    include_directories: [
        libhighlight_include,
    ],

    install: false,
    c_args: c_args,
    dependencies: libhightlight_dependencies,
)
