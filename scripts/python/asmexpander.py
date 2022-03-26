#!/usr/bin/env python3

''' Recursively expand assembler macros for ease of debugging '''

import argparse
import os
import re

from wrhandle import FileWrHandle, StdoutWrHandle

class Macro():
    ''' Macro representation '''

    def __init__(self, decl, definition):
        mgrp = re.match(r'\.macro\s(\w+)(\s+(.*))?$', decl)
        self._name = mgrp.group(1)
        self._def = definition.copy()
        if mgrp.group(2) is None:
            self._params = []
        else:
            self._params = list(map(lambda s: s.strip(), mgrp.group(2).split(',')))

    def expand(self, args):
        ''' Expand the macro given the appropriate args '''
        if len(args) != len(self._params):
            raise RuntimeError(f'Expected {len(self._params)} arguments, got {len(args)}')

        expanded = []
        for line in self.definition:
            for paramname, arg in zip(self._params, args):
                if paramname in line:
                    line = line.replace(f'\\{paramname}', arg)
            expanded.append(line)

        return expanded

    @property
    def name(self):
        ''' Get the name of the macro '''
        return self._name

    @property
    def definition(self):
        ''' Get the macro definition (unsubstituted) '''
        return self._def

    @property
    def params(self):
        ''' Get the parameter names '''
        return self._params

class DoubleBuffer():
    ''' Double buffer abstraction '''
    def __init__(self, initial=None):
        self._fbuf = initial if initial is not None else []
        self._bbuf = []
        self._front = self._fbuf
        self._back = self._bbuf

    def swap(self):
        ''' Swap front and back buffers '''
        tmp = self._front
        self._front = self._back
        self._back = tmp

    @property
    def front(self):
        ''' Access front buffer '''
        return self._front

    @property
    def back(self):
        ''' Access back buffer '''
        return self._back

    def append_back(self, arg):
        ''' Append to back buffer '''
        if not isinstance(arg, list):
            self._back.append(arg)
        else:
            self._back += arg

def expand_cpp_includes(file, content, verbose):
    ''' Include cpp-stype #includes '''
    def include_file(filename, fdir):
        path = None
        for fpath in [filename, os.path.join(fdir, filename)]:
            if verbose:
                print(f'Trying {fpath}... ', end='')
            if os.path.exists(fpath):
                path = fpath
                if verbose:
                    print('ok')
                break

            if verbose:
                print('not found')

        if path is None:
            raise FileNotFoundError(f'Could not find {filename}')

        with open(path, 'r', encoding='ascii') as handle:
            return [s.rstrip() for s in handle.readlines()]

    processed = -1

    # Check same dir as source file
    fdir = os.path.dirname(file)

    dbuf = DoubleBuffer(content)

    while processed != 0:
        processed = 0
        for line in dbuf.front:
            if line.startswith('#include'):
                filename = re.sub(r'#include\s*["<]([^ ]+)[">]', r'\1', line)
                if verbose:
                    print(f'Expanding {filename}')
                dbuf.append_back(include_file(filename, fdir))
                processed += 1
            else:
                dbuf.append_back(line)
                continue
        dbuf.swap()
        dbuf.back.clear()

    return dbuf.front

def parse_macros(content, verbose):
    ''' Parse macro definitions from the assembly file '''
    macros = []
    current = []
    in_macro = False
    decl = None
    decllines = []

    lineno = 0
    for line in content:
        lineno += 1
        if line.startswith('.macro'):
            decl = line
            decllines.append(lineno)
            in_macro = True
        elif line.startswith('.endm'):
            in_macro = False
            macros.append(Macro(decl, current))
            current.clear()
        elif in_macro:
            current.append(line)

    if verbose:
        for mac, line in zip(macros, decllines):
            print(f'Parsed macro {mac.name} on line {line}')

    return macros

def expand_all(macros, content, verbose):
    ''' Expand all macro invocations '''
    regex = re.compile(r'\s*(\w+)(\s*([^#]+))?.*')

    dbuf = DoubleBuffer(content)

    for macro in macros:
        if verbose:
            print(f'Expanding {macro.name}')

        for line in dbuf.front:
            if not line:
                dbuf.append_back(line)
                continue

            mat = regex.match(line)
            if mat is None:
                dbuf.append_back(line)
                continue

            ins = mat.group(1)
            if ins != macro.name:
                dbuf.append_back(line)
                continue

            args = mat.group(3)
            if args is None:
                args = []
            else:
                args = list(filter(lambda s: s, map(lambda s: s.strip(), args.split(','))))

            dbuf.append_back(macro.expand(args))

        expanded = parse_macros(dbuf.back, verbose)
        for exp in expanded:
            for j, dmacro in enumerate(macros):
                if dmacro.name == exp.name:
                    macros[j] = exp

        dbuf.swap()
        dbuf.back.clear()

    return dbuf.front

def strip_macros(content, verbose):
    ''' Strip macro definitions '''
    stripped = []
    skip = False
    for line in content:
        if line.startswith('.macro'):
            skip = True
            if verbose:
                print('Stripping macro {}'.format(re.sub(r"\s+", " ", line).split()[1]))
        elif line.startswith('.endm'):
            skip = False
            continue

        if skip:
            continue

        stripped.append(line)

    return stripped

def eval_conditionals(content):
    ''' Evaluate constant condition conditionals '''
    ifxpr = None
    elsexpr = None
    endifxpr = None
    conditionals = []
    for i, line in enumerate(content):
        if line.startswith('.if'):
            ifxpr = i
        elif line.startswith('.else'):
            elsexpr = i
        elif line.startswith('.endif'):
            endifxpr = i

        if endifxpr is not None:
            conditionals.append((ifxpr, elsexpr, endifxpr))
            ifxpr = None
            elsexpr = None
            endifxpr = None

    for cond in reversed(conditionals):
        ifxpr, elsexpr, endifxpr = cond
        elsexpr = elsexpr if elsexpr is not None else endifxpr
        if content[ifxpr].endswith('1'):
            del content[elsexpr:endifxpr+1]
            del content[ifxpr]
        else:
            del content[endifxpr]
            del content[ifxpr:elsexpr+1]

    return content

def main(file, outfile, verbose):
    ''' Main function '''
    with open(file, 'r', encoding='ascii') as handle:
        content = list(map(lambda s: s.rstrip(), handle.readlines()))
    content = expand_cpp_includes(file, content, verbose)
    macros = parse_macros(content, verbose)
    expanded = expand_all(macros, content, verbose)
    stripped = strip_macros(expanded, verbose)
    final = eval_conditionals(stripped)

    with FileWrHandle(outfile) if outfile is not None else StdoutWrHandle() as handle:
        for line in final:
            handle.writeln(line)

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Recursively expand assember macros')
    parser.add_argument('file', metavar='FILE', type=str, help='Path to the assembly file')
    parser.add_argument('-o', '--outfile', type=str, default=None, help='Desired output file')
    parser.add_argument('-v', '--verbose', action='store_true', help='Enable verbose output')
    main(**vars(parser.parse_args()))
