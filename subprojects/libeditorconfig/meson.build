project('libeditorconfig', 'c')

subdir('src')

subdir('include')

libeditorconfig_dependencies = []

c_args = []

libeditorconfig = static_library(
    'editorconfig',
    libeditorconfig_sources,
    include_directories: [
        libeditorconfig_include,
    ],

    install: false,
    c_args: c_args,
    dependencies: libeditorconfig_dependencies,
)

libeditorconfig_dep = declare_dependency(
    include_directories: [
        libeditorconfig_include,
    ],
    link_with: libeditorconfig,
)

if get_option('test')
    subdir('test')
endif
