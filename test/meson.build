#subdir('integration')

e_test = ['list.c', 'message.c']

testlib_dep = dependency('testlib')
foreach p : e_test
    sources = [p] + e_src
    t = executable(
        p.underscorify(),
        sources,
        install: false,
        include_directories: [
            e_include,
        ],
        dependencies: [
            testlib_dep,
            #libhighlight_dep,
            librope_dep,
            cextras_dep,
        ],
    )
    test(p, t, timeout: 1)
endforeach
