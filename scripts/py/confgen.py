#!/usr/bin/python
''' Generates configuration header to store library configuration persistently '''

import argparse
import re

from wrhandle import FileWrHandle, StdoutWrHandle

def load_configs(configs):
    ''' Load and parse configrations (.mk format) '''
    data = {}
    pat = re.compile(r'^\s*([^ ]+)\s*:?=\s*([^ ]+)\s*$')
    for conf in configs:
        with open(conf, 'r', encoding='ascii') as handle:
            content = handle.read()

        for line in content.split('\n'):
            if (match := pat.match(line)) is None:
                continue

            data[match.group(1)] = match.group(2)

    return data

def write(data, semver, outfile, enable_simd):
    ''' Write the header '''
    formats = {
        'arch': 'SCC_ARCH_{}',
        'bitarch': 'SCC_{}BIT',
        'simd_isa': 'SCC_SIMD_ISA_{}',
        'hostos': 'SCC_HOST_OS_{}',
        'vecsize': 'SCC_VECSIZE {}',
        'abi': 'SCC_ABI_{}',
        'CONFIG_ARENA_CHUNKSIZE': 'SCC_ARENA_CHUNKSIZE {}'
    }
    major, minor, patch = semver.split('.')
    with FileWrHandle(outfile) if outfile is not None else StdoutWrHandle() as handle:
        handle.writeln('#ifndef SCC_CONFIG_H')
        handle.writeln('#define SCC_CONFIG_H\n')

        handle.writeln('#ifndef SCC_VERSION_MAJOR')
        handle.writeln(f'#define SCC_VERSION_MAJOR {major}')
        handle.writeln('#endif\n')

        handle.writeln('#ifndef SCC_VERSION_MINOR')
        handle.writeln(f'#define SCC_VERSION_MINOR {minor}')
        handle.writeln('#endif\n')

        handle.writeln('#ifndef SCC_VERSION_PATCH')
        handle.writeln(f'#define SCC_VERSION_PATCH {patch}')
        handle.writeln('#endif\n')

        if enable_simd:
            handle.writeln('#ifndef SCC_SIMD_ENABLED')
            handle.writeln('#define SCC_SIMD_ENABLED')
            handle.writeln('#endif\n')

        for var, fmt in formats.items():
            try:
                name = fmt.format(data[var]).split(' ', maxsplit=1)[0]
                handle.writeln(f'#ifndef {name}')
                handle.writeln(f'#define {fmt.format(data[var])}')
                handle.writeln('#endif\n')
            except KeyError as err:
                if not str(err).replace("'","").startswith('CONFIG_'):
                    raise
        handle.writeln('#endif /* SCC_CONFIG_H */')

def main(configs, semver, outfile, enable_simd):
    ''' Main '''
    data = load_configs(configs)
    write(data, semver, outfile, enable_simd)

if __name__ == '__main__':
    parser = argparse.ArgumentParser(
            description='Generate configuration header')
    parser.add_argument('-o', '--outfile', action='store', default=None,
        help='Path to write the generated header document to')
    parser.add_argument('-s', '--enable-simd', help='Enable SIMD support', action='store_true')
    parser.add_argument('semver', metavar='SEMVER', help='Semantic version')
    parser.add_argument('configs', nargs='+', help='Input configuration files')
    main(**vars(parser.parse_args()))
