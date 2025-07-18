e_test = [
    'match.c',
]

testlib_dep = dependency('testlib')
foreach p : e_test
    sources = [p]
    t = executable(
        p.underscorify(),
        sources,
        install: false,
        dependencies: [
            libeditorconfig_dep,
            testlib_dep,
        ],
    )
    test(p, t)
endforeach
