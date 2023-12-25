e_test = [
    'librope/librope.c',
    'librope/cursor.c',
    'librope/range.c',
    'librope/node.c',
]

testlib_dep = subproject('cextras').get_variable('testlib_dep')
foreach p : e_test
    sources = [p]
    t = executable(
        p.underscorify(),
        sources,
        install: false,
        link_with: [librope],
        dependencies: [testlib_dep],
        include_directories: [librope_include],
    )
    test(p, t)
endforeach
