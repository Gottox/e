subdir('src')

subdir('include')

lsp_gen_model = executable(
    'lsp_gen_model',
    [lsp_gen_model_src],
    include_directories: lsp_gen_model_include,
    dependencies: [cextras_dep, libjsonwrap_dep],
)
