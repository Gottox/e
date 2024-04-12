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
        link_with: [librope, libhighlight],
        dependencies: [utest_dep],
        include_directories: [librope_include, libhighlight_include],
    )
    test(p, t)
endforeach
