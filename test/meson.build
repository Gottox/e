librope_test = files('librope/librope.c')

testlib_dep = subproject('cextras').get_variable('testlib_dep')
test = executable(
    'librope_test',
    librope_test,
    install: false,
    link_with: [librope],
    dependencies: [testlib_dep],
    include_directories: [librope_include],
)
test('librope', test)
