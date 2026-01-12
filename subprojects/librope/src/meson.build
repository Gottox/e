librope_sources = files(
    'cursor.c',
    'iterator.c',
    'node/navigation.c',
    'node/mutation.c',
    'node/info.c',
    'node/tags.c',
    'node/node.c',
    'pool.c',
    'range.c',
    'rc_string.c',
    'rope.c',
)

if jsonc_dep.found()
    librope_sources += files('serialization/json.c')
endif
if get_option('serialization').disabled() == false
    librope_sources += files('serialization/graphviz.c')
endif
