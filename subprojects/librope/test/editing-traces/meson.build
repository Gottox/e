sequential_traces = subproject('editing-traces').get_variable('sequential_traces')
#concurrent_traces = subproject('editing-traces').get_variable('concurrent_traces')

cjson_dep = dependency('json-c')
editing_traces = executable(
    'editing-traces',
    'editing-traces.c',
    install: false,
    dependencies: [testlib_dep, librope_dep, cjson_dep],
)
foreach trace : sequential_traces
    args = []
    test_name = trace.full_path().split('/')[-1].replace('.json', '')
    args += trace.full_path()
    test(
        test_name,
        editing_traces,
        args: args,
        depends: trace,
    )
endforeach
