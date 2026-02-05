# Gen3: LSP code generator with Intermediate Representation

# IR Generator
lsp_ir_gen = executable(
    'lsp_ir_gen',
    ['ir_gen.c', 'common.c'],
    include_directories: include_directories('.'),
    dependencies: [jsonc_dep],
)

# Code Generator
lsp_code_gen = executable(
    'lsp_code_gen',
    ['code_gen.c', 'common.c'],
    include_directories: include_directories('.'),
    dependencies: [jsonc_dep],
)

# Generate IR from metaModel.json
lsp_ir = custom_target('lsp_ir',
    output: 'ir.json',
    input: 'metaModel.json',
    capture: true,
    command: [lsp_ir_gen, '@INPUT@'],
)

# Generate model.h/model.c from ir.json
clang_format = find_program('clang-format', required: false)
liblsp_gen_env = {}
if clang_format.found()
    liblsp_gen_env = {'CLANG_FORMAT': clang_format.full_path()}
endif

liblsp_gen_model = custom_target('lsp_gen_model',
    output: ['model.h', 'model.c'],
    input: lsp_ir,
    env: liblsp_gen_env,
    command: [lsp_code_gen, '@INPUT@', '@OUTPUT0@', '@OUTPUT1@'],
)

# Message Generator (requests + notifications)
lsp_msg_gen = executable(
    'lsp_msg_gen',
    ['msg_gen.c', 'common.c'],
    include_directories: include_directories('.'),
    dependencies: [jsonc_dep],
)

# Generate messages.h/messages.c from ir.json
liblsp_gen_messages = custom_target('lsp_gen_messages',
    output: ['messages.h', 'messages.c'],
    input: lsp_ir,
    env: liblsp_gen_env,
    command: [lsp_msg_gen, '@INPUT@', '@OUTPUT0@', '@OUTPUT1@'],
)

# Export generated sources and include directory
liblsp_gen_sources = [liblsp_gen_model, liblsp_gen_messages]
liblsp_gen_include = include_directories('.')
