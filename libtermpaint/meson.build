subdir('src')

subdir('include')

libtermpaint_dependencies = [cextras_dep]

c_args = []

libtermpaint = static_library(
    'rope',
    libtermpaint_sources,
    include_directories: [
        libtermpaint_include,
    ],

    install: false,
    c_args: c_args,
    dependencies: libtermpaint_dependencies,
)
