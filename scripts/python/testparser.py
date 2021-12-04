#!/usr/bin/env python3

''' Hack for generating valid C programs for each panic test '''

import argparse
import os
import re

def find_functions(infile):
    ''' Find test functions in source file '''
    with open(infile, 'r', encoding='ascii') as handle:
        contents = handle.read()

    funcs = []
    for line in contents.split('\n'):
        if (match := re.match(r'\s*(void\s+([^ ]+)\(void\))\s*{', line)) is not None:
            funcs.append((match.groups()[1], match.groups()[0]))
    return funcs

def generate_runners(funcs, outdir):
    ''' Generate a file with main invoking the test for each function '''
    written_files = []
    for func, decl in funcs:
        contents = [f'extern {decl};',
                     'int main(void) {',
                    f'{4 * " "}{func}();',
                     '}']

        if outdir is None:
            print(f'{func}.c:')
            print('\n'.join(contents))
            continue

        sfile = os.path.join(outdir, f'{func}.c')
        with open(sfile, 'w', encoding='ascii') as handle:
            handle.write('{}\n'.format('\n'.join(contents)))

        written_files.append(sfile)

    return written_files

def generate_makelist(outdir, written, makevar):
    ''' Create list of generated files and write it to makefile '''
    makevar = makevar if makevar is not None else 'generated_testfiles'
    manifest = os.path.join(outdir, 'manifest.mk')
    contents = {}
    if os.path.isfile(manifest):
        with open(manifest, 'r', encoding='ascii') as handle:
            existing = handle.read()

        for line in existing.split('\n')[:-1]:
            var, value = tuple(map(lambda s: s.strip(), line.split(':=')))
            if var not in contents:
                contents[var] = set()

            for val in list(map(lambda s: s.strip(), value.split(' '))):
                contents[var].add(val)

    if makevar not in contents:
        contents[makevar] = set()
    for file in written:
        contents[makevar].add(file)

    with open(os.path.join(outdir, 'manifest.mk'), 'w', encoding='ascii') as handle:
        for var, val in contents.items():
            handle.write(f'{var} := {" ".join(val)}\n')

def main(infile, outdir, makevar):
    ''' Find functions in source file, generate runners and write makefile '''
    funcs = find_functions(infile)
    written = generate_runners(funcs, outdir)
    if outdir is not None:
        generate_makelist(outdir, written, makevar)

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Generate compilable C source files from snippets')
    parser.add_argument('infile', type=str, help='Input snippet')
    parser.add_argument('-o', '--outdir', type=str, default=None, help='Output directory')
    parser.add_argument('-m', '--makevar', default=None, help='make variable to assign list to')
    main(**vars(parser.parse_args()))
