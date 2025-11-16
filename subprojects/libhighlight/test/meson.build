e_test = ['libhighlight.c']

testlib_dep = dependency('testlib')
foreach p : e_test
    sources = files(
        'tree-sitter-markdown-inline/src/parser.c',
        'tree-sitter-markdown-inline/src/scanner.c',
        p,
    )
    t = executable(
        p.underscorify(),
        sources,
        install: false,
        dependencies: [
            testlib_dep,
            libhighlight_dep,
            cextras_dep,
        ],
    )
    test(p, t)
endforeach
