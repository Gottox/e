sh = find_program('sh')
e_integration_test = ['']

foreach p : e_integration_test
    test(p, sh, args: [p], timeout: 5)
endforeach

