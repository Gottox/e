subdir('src')

subdir('include')

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
