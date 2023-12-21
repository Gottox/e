project(
    'e',
    'c',
    version: '0.1',
    default_options: [
        'warning_level=3',
        'quickjs:warning_level=2',
        'quickjs:werror=false',
    ],
)

quickjs = dependency('quickjs')
cextras = dependency('cextras')
json_c = dependency('json-c')
#tree_sitter = dependency('tree-sitter')
#tree_sitter = subproject('tree-sitter').get_variable('tree_sitter_dep')

dependencies = [
    quickjs,
    cextras,
    #tree_sitter,
]

subdir('src')

subdir('include')

subdir('librope')

exe = executable(
    'e',
    e_src,
    include_directories: [
        e_include,
        librope_include,
    ],
    link_with: [
        librope,
    ],
    dependencies: dependencies,
    install: true,
)

subdir('test')
