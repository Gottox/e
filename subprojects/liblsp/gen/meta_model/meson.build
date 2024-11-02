subdir('src')

subdir('include')

metamodel_gen = executable(
    'metamodel_gen',
    [metamodel_gen_src],
    include_directories: include_directories('include'),
    dependencies: [quickjs_dep, cextras_dep, libjsonwrap_dep],
)
