testlib_dep = subproject('cextras').get_variable('testlib_dep')
test_src = ['lsp_connection.c', 'requests.c', 'notifications.c', 'lsp_util.c', 'lsp_server_test.c', 'lsp_client_test.c', 'initialization.c']

#subdir('gen')

foreach src : test_src
    test_exe = executable(
        'test-' + src.underscorify(),
        src,
        dependencies: [testlib_dep, liblsp_dep],
        install: true,
        install_dir: join_paths(get_option('bindir'), 'tests'),
    )
    test(src.underscorify(), test_exe, args: ['-t'], protocol: 'tap')
endforeach

# Integration test (requires clangd)
integration_exe = executable(
    'test-integration_c',
    'integration.c',
    dependencies: [testlib_dep, liblsp_dep],
    install: false,
)
test('integration_c', integration_exe, timeout: 30)
