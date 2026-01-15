jsonc_dep = dependency('json-c', default_options: ['warning_level=0', 'b_sanitize='])

testlib_dep = dependency('testlib')

#subdir('editing-traces')

e_test_extra_src = files(
    '../fuzzer/deser.c',
)
e_test = [
    'cursor.c',
    'fuzzer_repro.c',
    'iterator.c',
    'librope.c',
    'node.c',
    'range.c',
    'str.c',
]

foreach p : e_test
    sources = [p] + e_test_extra_src
    t = executable(
        p.underscorify(),
        sources,
        install: false,
        dependencies: [
            testlib_dep,
            librope_dep,
            cextras_dep,
            jsonc_dep,
        ],
    )
    test(p, t, args: ['-t'], protocol: 'tap')
endforeach
