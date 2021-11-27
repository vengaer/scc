#!/usr/bin/env python3

import argparse
import json
import re

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

def name_parameters(decl, params):
    named = []
    params = params.split(',')
    pre = re.compile(r'^(\s*\w+(\s+(const|volatile|const volatile|volatile const))?(\s+\*))?(.*)')
    for i, param in enumerate(params):
        m = pre.search(param)
        if len(m.group(5).replace(' ', '')) == 0:
            named.append(f'{m.group(1)} param{i}')
        else:
            named.append(param)

    m = re.search(r'^([^(]+)\(.*\)(.*)', decl)
    return f'{m.group(1)}(' + ','.join(named) + f'){m.group(2)}'

def define_externs(func_lines):
    lines = []
    exre = re.compile(r'^\s*extern\s+((\w+)\s+.+\((.+)\)).*;')
    for line in func_lines:
        if (m := exre.search(line)):
            lines.append('{} {{'.format(name_parameters(m.group(1), m.group(3))))
            lines.append(' ' * 4 + f'return ({m.group(2)}){{ 0 }};')
            lines.append('}')
        else:
            lines.append(line)
    return lines

def genfile(outfile, headers, snip):
    func_re = re.compile(r'^\s*((extern|static)\s+)?\w+\s+[a-zA-Z_][a-zA-Z_0-9]*\([^)]+\)')
    func_lines = []
    nonfunc_lines = []

    in_function = False
    balance = 0
    for line in snip.split('\n'):
        if func_re.search(line):
            in_function = True
        if in_function:
            balance += line.count('{') - line.count('}')
            if balance <= 0:
                in_function = False
            func_lines.append(line)
        else:
            nonfunc_lines.append(line)

    func_lines = define_externs(func_lines)

    with open(outfile, 'w') as fp:
        fp.write('{}\n\n'.format('\n'.join([f'#include <{h}.h>' for h in headers])))
        fp.write('{}\n\n'.format('\n'.join(func_lines)))
        fp.write('int main(void) {')
        fp.write('\n'.join(f'{4*" "}{line}' for line in nonfunc_lines[:-1]))
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
