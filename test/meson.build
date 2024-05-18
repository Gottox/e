e_test = [
    'librope/librope.c',
    'librope/cursor.c',
    'librope/range.c',
    'librope/node.c',
    'libhighlight/highlight.c',
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
            e_parsers_dep,
            libhighlight_dep,
            librope_dep,
            cextras_dep,
        ],
    )
    test(p, t)
endforeach
