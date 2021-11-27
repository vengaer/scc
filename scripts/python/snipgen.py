#!/usr/bin/env python3

import argparse

_symmap = {
    'scc/scc_vec': [
        'scc_vec_init',
        'scc_vec_from',
        'scc_vec_push',
        'scc_vec_free',
        'scc_vec_size',
        'scc_vec_reserve',
        'scc_vec_erase',
        'scc_vec_erase_range',
        'scc_vec_foreach'
        'scc_vec_foreach_reversed'
        'scc_vec_foreach_by'
        'scc_vec_foreach_reversed_by'
    ],
    'scc/scc_svec': [
        'scc_svec_init',
        'scc_svec_free',
        'scc_svec_reserve',
        'scc_svec_push',
        'scc_svec_from',
        'scc_svec_erase',
        'scc_svec_erase_range'
    ],
    'scc/scc_rbtree': [
        'scc_rbtree_init',
        'scc_rbtree_free',
        'scc_rbtree_insert',
        'scc_rbtree_find',
        'scc_rbtree_remove',
        'scc_rbtree_foreach',
        'scc_rbtree_foreach_reversed'
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
    'string': [
        'strlen'
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
        fp.write('\n'.join(f'{4*" "}{line}' for line in snip.split('\n')[:-1]))
        fp.write('\n}\n')

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
