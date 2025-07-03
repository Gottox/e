e_test = [
    'librope.c',
    'cursor.c',
    'range.c',
    'node.c',
]

utest_dep = dependency('utest')
foreach p : e_test
    sources = [p]
    t = executable(
        p.underscorify(),
        sources,
        install: false,
        dependencies: [
            utest_dep,
            librope_dep,
            cextras_dep,
        ],
    )
    test(p, t)
endforeach
