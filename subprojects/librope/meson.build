project('librope', 'c', default_options: ['c_std=c11', 'warning_level=3'])

cextras_dep = dependency('cextras')
libgrapheme_dep = dependency('libgrapheme')
jsonc_dep = dependency('json-c', required: get_option('serialization'))

librope_dependencies = [cextras_dep, libgrapheme_dep, jsonc_dep]

subdir('src')

subdir('include')

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
    dependencies: librope_dependencies,
)

if get_option('test')
    subdir('test')
endif

if get_option('fuzzer')
    subdir('fuzzer')
endif
