project('librope', 'c', default_options: ['c_std=c11', 'warning_level=3'])

cextras_dep = dependency('cextras', default_options: ['warning_level=0'])
libgrapheme_dep = dependency(
    'libgrapheme',
    default_options: ['test=false', 'benchmark=false', 'werror=false', 'buildtype=release', 'b_lto=true'],
)
libpcre2_dep = dependency('libpcre2-8')

librope_dependencies = [cextras_dep, libgrapheme_dep, libpcre2_dep]

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
