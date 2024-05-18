project('liblsp', 'c')

subdir('src')

subdir('include')

quickjs_dep = subproject('quickjs').get_variable('quickjs_dep')

liblsp_dependencies = [quickjs_dep]

c_args = []

liblsp = static_library(
    'lsp',
    liblsp_sources,
    include_directories: [
        liblsp_include,
    ],

    install: false,
    c_args: c_args,
    dependencies: liblsp_dependencies,
)

liblsp_dep = declare_dependency(
    include_directories: [
        liblsp_include,
    ],
    link_with: liblsp,
)
