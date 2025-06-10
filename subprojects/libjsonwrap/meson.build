project('libjsonwrap', 'c')

cextras_dep = subproject('cextras').get_variable('cextras_dep')

subdir('src')

subdir('include')

if get_option('test')
    subdir('test')
endif

libjsonwrap_dependencies = [cextras_dep]
if get_option('json_backend') == 'quickjs'
    backend_dependencies = [dependency('quickjs-ng')]
endif

c_args = []

libjsonwrap = static_library(
    'jsonwrap',
    libjsonwrap_sources,
    include_directories: [
        libjsonwrap_include,
    ],

    install: false,
    c_args: c_args,
    dependencies: libjsonwrap_dependencies + backend_dependencies,
)

libjsonwrap_dep = declare_dependency(
    include_directories: [
        libjsonwrap_include,
    ],
    link_with: libjsonwrap,
    dependencies: backend_dependencies
)
