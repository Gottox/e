sequential_traces = subproject('editing-traces').get_variable('sequential_traces')
concurrent_traces = subproject('editing-traces').get_variable('concurrent_traces')

cjson_dep = dependency('json-c')
t = executable(
    'editing-traces',
    'editing-traces.c',
    install: false,
    dependencies: [testlib_dep, librope_dep, cjson_dep],
)
foreach trace : sequential_traces
    args = []
    if trace.full_path().contains('_ascii_only_')
        args += '-i'
    endif
    args += [trace.full_path()]
    test(
        trace.full_path().split('/')[-1].replace('.json', ''),
        t,
        args: args,
        timeout: 1800,
        depends: trace,
    )
endforeach
