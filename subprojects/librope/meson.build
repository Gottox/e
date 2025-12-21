project('librope', 'c', default_options: ['c_std=c11'])

subdir('src')

subdir('include')

cextras_dep = dependency('cextras')
libgrapheme_dep = dependency('libgrapheme')

librope_dependencies = [cextras_dep, libgrapheme_dep]

c_args = []
#c_args += ['-DROPE_THICK_NODES']
if get_option('single_line_nodes')
    c_args += ['-DROPE_SINGLE_LINE_NODES']
endif

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
