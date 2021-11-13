#!/usr/bin/env python3

import argparse

_symmap = {
    'scc_vec_init': 'scc/scc_vec',
    'scc_vec_push': 'scc/scc_vec',
    'scc_vec_free': 'scc/scc_vec',
    'scc_vec_size': 'scc/scc_vec',
    'scc_vec_reserve': 'scc/scc_vec',
    'fgets': 'stdio',
    'fputs': 'stdio',
    'puts': 'stdio',
    'exit': 'stdlib',
    'strtoul': 'stdlib',
    'isspace': 'ctype'
}

def required_headers(snip):
    headers = set()
    for func, header in _symmap.items():
        if func in snip:
            headers.add(header)
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
