#!/usr/bin/env python3

''' Turn documentation snippets into valid C programs '''

import argparse
import json
import re

def required_headers(snip, syms):
    ''' Generate a set of headers to include '''
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
    ''' Name unnamed parameters. Used when defining functions declared extern '''
    named = []
    params = params.split(',')
    pre = re.compile(r'^(\s*\w+(\s+(const|volatile|const volatile|volatile const))?(\s+\*))?(.*)')
    for i, param in enumerate(params):
        if len((match := pre.search(param)).group(5).replace(' ', '')) == 0:
            named.append(f'{match.group(1)} param{i}')
        else:
            named.append(param)

    match = re.search(r'^([^(]+)\(.*\)(.*)', decl)
    return f'{match.group(1)}(' + ','.join(named) + f'){match.group(2)}'

def define_externs(func_lines, funcs):
    ''' Define functions declared as extern '''
    lines = []
    exre = re.compile(r'^\s*extern\s+((.*)\s+([a-zA-Z_0-9]+)\((.+)\)).*;')
    for line in func_lines:
        if (match := exre.search(line)):
            if (funcname := match.group(3)) in funcs:
                lines.append(f'{funcs[funcname]["return"]} {funcname}'
                             f'({", ".join(funcs[funcname]["params"])}) {{')
                for funcline in funcs[funcname]['body']:
                    lines.append(' ' * 4 + funcline)
                lines.append('}')
            else:
                lines.append(f'{name_parameters(match.group(1), match.group(4))} {{')
                lines.append(' ' * 4 + f'return ({match.group(2)}){{ 0 }};')
                lines.append('}')
        else:
            lines.append(line)
    return lines

def genfile(outfile, headers, snip, funcs):
    ''' Generate valid C file snippet '''
    func_re = re.compile(r'^\s*((extern|static)\s+)?\w+\s+[a-zA-Z_][a-zA-Z_0-9]*\([^)]+\)')
    func_lines = []
    nonfunc_lines = []
    explicit_includes = []

    in_function = False
    balance = 0
    for line in snip.split('\n'):
        if line.startswith('#include'):
            explicit_includes.append(line)
            continue
        if func_re.search(line):
            in_function = True
        if in_function:
            balance += line.count('{') - line.count('}')
            if balance <= 0:
                in_function = False
            func_lines.append(line)
        else:
            nonfunc_lines.append(line)

    func_lines = define_externs(func_lines, funcs)

    with open(outfile, 'w', encoding='ascii') as handle:
        handle.write('{}\n'.format('\n'.join(explicit_includes)))
        handle.write('{}\n\n'.format('\n'.join([f'#include <{h}.h>' for h in headers])))
        handle.write('{}\n\n'.format('\n'.join(func_lines)))
        handle.write('int main(void) {\n')
        handle.write('\n'.join(f'{4*" "}{line}' for line in nonfunc_lines[:-1]))
        handle.write('\n}\n')

def main(infile, outfile, symmap, functions):
    ''' Read files, determine required headers and pass everything to genfile '''
    outfile = outfile if outfile is not None else 'a.c'

    with open(infile, 'r', encoding='ascii') as handle:
        snip = handle.read()

    with open(symmap, 'r', encoding='ascii') as handle:
        syms = json.load(handle)

    with open(functions, 'r', encoding='ascii') as handle:
        funcs = json.load(handle)

    headers = required_headers(snip, syms)
    genfile(outfile, headers, snip, funcs)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Generate compilable C source files from snippets')
    parser.add_argument('infile', type=str, help='Input snippet')
    parser.add_argument('-o', '--outfile', type=str, default=None, help='Output file')
    parser.add_argument('-s', '--symmap', type=str, default=None,
                        help='JSON file mapping headers to declarations')
    parser.add_argument('-f', '--functions', type=str, default=None,
                        help='JSON file with function definitions')
    main(**vars(parser.parse_args()))
