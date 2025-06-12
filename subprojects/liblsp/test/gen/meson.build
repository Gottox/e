lsp_gen_tests = [
    'utils_test.c',
    'references_test.c',
]

testlib_dep = subproject('cextras').get_variable('testlib_dep')

lsp_model_dir = '../../gen/lsp-model'

gen_sources = files(
  lsp_model_dir + '/src/utils.c',
  lsp_model_dir + '/src/collections.c',
  lsp_model_dir + '/src/resolve.c',
)

foreach src : lsp_gen_tests
  t = executable(
    src.underscorify(),
    [src, gen_sources],
    include_directories: [lsp_gen_model_include],
    dependencies: [testlib_dep, cextras_dep, libjsonwrap_dep],
    install: false,
  )
  test(src, t)
endforeach
