testlib_dep = dependency('testlib')

subdir('editing-traces')

e_test_extra_src = files(
    '../fuzzer/deser.c',
)
e_test = [
    'librope.c',
    'cursor.c',
    'range.c',
    'node.c',
    'iterator.c',
    'fuzzer_repro.c',
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
        ],
    )
    test(p, t)
endforeach
