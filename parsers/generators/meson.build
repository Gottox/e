# usage: embed_gen.process('input.txt', extra_args: 'var_name')
embed_exec = executable('embed', 'embed.c', native: true)
embed_gen = generator(
    embed_exec,
    arguments: ['@INPUT@', '@OUTPUT@', '@EXTRA_ARGS@'],
    output: '@BASENAME@.c',
)

build_parser_exec = executable('build_parser', 'build_parser.c', native: true)

parser_table_def_exec = executable(
    'build-parser-table-def',
    'build-parser-table-def.c',
    native: true,
)
