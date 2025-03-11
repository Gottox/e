project('libjsonwrap', 'c')

quickjs_dep = dependency('quickjs-ng')
cextras_dep = subproject('cextras').get_variable('cextras_dep')

subdir('src')

subdir('include')

if get_option('test')
    subdir('test')
endif

libjsonwrap_dependencies = [quickjs_dep, cextras_dep]

c_args = []

libjsonwrap = static_library(
    'jsonwrap',
    libjsonwrap_sources,
    include_directories: [
        libjsonwrap_include,
    ],

    install: false,
    c_args: c_args,
    dependencies: libjsonwrap_dependencies,
)

libjsonwrap_dep = declare_dependency(
    include_directories: [
        libjsonwrap_include,
    ],
    link_with: libjsonwrap,
)
