project('liblsp', 'c')

quickjs_dep = dependency('quickjs-ng')
jsonc_dep = dependency('json-c', default_options: ['warning_level=0', 'b_sanitize='])
#libjsonwrap_dep = subproject('libjsonwrap').get_variable('libjsonwrap_dep')
cextras_dep = subproject('cextras').get_variable('cextras_dep')
librandid_dep = subproject('librandid').get_variable('librandid_dep')

subdir('gen')

subdir('src')

subdir('include')


#liblsp_dependencies = [quickjs_dep, libjsonwrap_dep]
liblsp_dependencies = [jsonc_dep, librandid_dep, cextras_dep]

c_args = []

liblsp = static_library(
    'lsp',
    liblsp_sources + liblsp_gen_sources,
    include_directories: [
        liblsp_include,
        liblsp_gen_include,
    ],
    install: false,
    c_args: c_args,
    dependencies: liblsp_dependencies,
)

liblsp_dep = declare_dependency(
    include_directories: [
        liblsp_include,
        liblsp_gen_include,
    ],
    link_with: liblsp,
    dependencies: liblsp_dependencies,
    sources: liblsp_gen_sources,
)

if get_option('test')
    subdir('test')
endif

subdir('examples')
