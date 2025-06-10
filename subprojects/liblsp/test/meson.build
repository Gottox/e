testlib_dep = subproject('cextras').get_variable('testlib_dep')
test_src = ['lsp_connection.c']

foreach src : test_src
  test_exe = executable(
    'test-' + src.underscorify(),
    src,
    dependencies: [testlib_dep, liblsp_dep],
    install: true,
    install_dir: join_paths(get_option('bindir'), 'tests')
  )
  test(src.underscorify(), test_exe)
endforeach
