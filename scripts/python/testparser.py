#!/usr/bin/env python3

import argparse
import os
import re

def find_functions(infile):
    with open(infile, 'r') as fp:
        contents = fp.read()

    funcs = []
    for line in contents.split('\n'):
        m = re.match(r'\s*(void\s+([^ ]+)\(void\))\s*{', line)
        if m is not None:
            funcs.append((m.groups()[1], m.groups()[0]))
    return funcs

def generate_runners(funcs, outdir):
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
        with open(sfile, 'w') as fp:
            fp.write('\n'.join(contents))

        written_files.append(sfile)

    return written_files

def generate_makelist(outdir, written, makevar):
    makevar = makevar if makevar is not None else 'generated_testfiles'
    manifest = os.path.join(outdir, 'manifest.mk')
    contents = dict()
    if os.path.isfile(manifest):
        with open(manifest, 'r') as fp:
            existing = fp.read()

        for line in existing.split('\n')[:-1]:
            sp = tuple(map(lambda s: s.strip(), line.split(':=')))
            if sp[0] not in contents:
                contents[sp[0]] = set()

            for val in list(map(lambda s: s.strip(), sp[1].split(' '))):
                contents[sp[0]].add(val)

    if makevar not in contents:
        contents[makevar] = set()
    for f in written:
        contents[makevar].add(f)

    with open(os.path.join(outdir, 'manifest.mk'), 'w') as fp:
        for k, v in contents.items():
            fp.write(f'{k} := {" ".join(v)}\n')

def main(infile, outdir, makevar):
    funcs = find_functions(infile)
    written = generate_runners(funcs, outdir)
    if outdir is not None:
        generate_makelist(outdir, written, makevar)

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Generate compilable C source files from snippets')
    parser.add_argument('infile', type=str, help='Input snippet')
    parser.add_argument('-o', '--outdir', type=str, default=None, help='Output directory')
    parser.add_argument('-m', '--makevar', type=str, default=None, help='Name of make variable to assign list of generated files to')
    main(**vars(parser.parse_args()))
