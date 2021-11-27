#!/usr/bin/env python3

import argparse
import json

def required_headers(snip, syms):
    headers = set()
    lines = snip.split('\n')
    for header in syms:
        for func in syms[header]:
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

def main(infile, outfile, symmap):
    outfile = outfile if outfile is not None else 'a.c'

    with open(infile, 'r') as fp:
        snip = fp.read()

    with open(symmap, 'r') as fp:
        syms = json.load(fp)

    headers = required_headers(snip, syms)
    genfile(outfile, headers, snip)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Generate compilable C source files from snippets')
    parser.add_argument('infile', type=str, help='Input snippet')
    parser.add_argument('-o', '--outfile', type=str, default=None, help='Output file')
    parser.add_argument('-s', '--symmap', type=str, default=None, help='Path to JSON file mapping headers to symbols')
    main(**vars(parser.parse_args()))
