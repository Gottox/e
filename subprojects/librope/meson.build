project('librope', 'c')

subdir('src')

subdir('include')

cextras_dep = dependency('cextras')

librope_dependencies = [cextras_dep]

c_args = []

librope = static_library(
    'rope',
    librope_sources,
    include_directories: [
        librope_include,
    ],

    install: false,
    c_args: c_args,
    dependencies: librope_dependencies,
)

librope_dep = declare_dependency(
    include_directories: [librope_include],
    link_with: librope,
)

if get_option('test')
    subdir('test')
endif
