#!/usr/bin/env python3

import argparse

_symmap = {
    'scc/scc_vec': [
        'scc_vec_init',
        'scc_vec_push',
        'scc_vec_free',
        'scc_vec_size',
        'scc_vec_reserve',
        'scc_vec_erase',
        'scc_vec_foreach'
        'scc_vec_foreach_reversed'
        'scc_vec_foreach_by'
        'scc_vec_foreach_reversed_by'
    ],
    'stdio': [
        'fgets',
        'fputs',
        'puts',
        'printf'
    ],
    'stdlib': [
        'exit',
        'strtoul'
    ],
    'ctype': [
        'isspace'
    ]
}

def required_headers(snip):
    headers = set()
    lines = snip.split('\n')
    for header in _symmap:
        for func in _symmap[header]:
            for line in lines:
                if f'{func}(' in line:
                    headers.add(header)
                    break
    return headers

def genfile(outfile, headers, snip):
    with open(outfile, 'w') as fp:
        fp.write('{}\n\n'.format('\n'.join([f'#include <{h}.h>' for h in headers])))
        fp.write('int main(void) {\n')
        fp.write('\n'.join(f'{4*" "}{line}' for line in snip.split('\n')))
        fp.write('\r}\n')

def main(infile, outfile):
    outfile = outfile if outfile is not None else 'a.c'

    with open(infile, 'r') as fp:
        snip = fp.read()

    headers = required_headers(snip)
    genfile(outfile, headers, snip)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Generate compilable C source files from snippets')
    parser.add_argument('infile', type=str, help='Input snippet')
    parser.add_argument('-o', '--outfile', type=str, default=None, help='Output file')
    main(**vars(parser.parse_args()))
