add_languages('cpp', native: true)

git_prog = find_program('git', required: true)
npm_prog = find_program('npm', required: true)
tree_sitter_prog = find_program('tree-sitter', required: true)

tree_sitter_parsers_source = []

e_grammar_repositories = {}
foreach name, source : e_external_grammars
    name = source['git'].underscorify() + '-' + source['rev'].underscorify()
    e_grammar_repositories += {
        name: source,
    }
endforeach

e_grammar_repository_targets = {}
# Fetch external grammars
foreach name, source : e_grammar_repositories
    fetch_repository = custom_target(
        name,
        command: [
            './fetch_repository.sh',
            source['git'],
            source['rev'],
            '@OUTPUT@',
        ],
        env: {
            'GIT': git_prog.full_path(),
        },
        output: name,
    )
    run_npm = custom_target(
        name + '-run_npm',
        command: [
            './run_npm.sh',
            '@INPUT@',
            '@OUTPUT@',
        ],
        env: {
            'NPM': npm_prog.full_path(),
        },
        depends: fetch_repository,
        input: fetch_repository,
        output: name + '-npm_installed',
    )
    targets = [fetch_repository]
    if source.has_key('run_npm') and source['run_npm'] == 'root'
        targets += run_npm
    endif
    e_grammar_repository_targets += {
        source['git']: targets,
    }
endforeach

# build parsers from grammars
foreach name, source : e_external_grammars
    depends = []
    depends += e_grammar_repository_targets[source['git']]
    sub_path = ''
    if source.has_key('path')
        sub_path = '/' + source['path']
    endif
    cc = ' '.join(meson.get_compiler('c', native: true).cmd_array())
    cxx = ' '.join(meson.get_compiler('cpp', native: true).cmd_array())
    tree_sitter_parser = custom_target(
        name,
        command: [
            './build_parser.sh',
            name,
            '@INPUT0@' + sub_path,
            '@OUTDIR@',
        ],
        env: {
            'CC': cc,
            'CXX': cxx,
        },
        input: depends,
        output: [
            name + '-scanner.c',
            name + '-parser.c',
        ],
    )
    tree_sitter_parsers_source += tree_sitter_parser
endforeach
