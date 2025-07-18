e_test = [
    'librope.c',
    'cursor.c',
    'range.c',
    'node.c',
    'iterator.c',
]

testlib_dep = dependency('testlib')
foreach p : e_test
    sources = [p]
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
