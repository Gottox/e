project('liblsp', 'c')

quickjs_dep = dependency('quickjs-ng')
libjsonwrap_dep = subproject('libjsonwrap').get_variable('libjsonwrap_dep')
cextras_dep = subproject('cextras').get_variable('cextras_dep')

subdir('src')

subdir('include')

subdir('gen')

liblsp_dependencies = [quickjs_dep, libjsonwrap_dep]

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

if get_option('test')
    subdir('test')
endif

