subdir('src')

subdir('include')

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
)

subdir('tinker')
