#!/usr/bin/env python3

''' Generate processor information given a SIMD ISA '''

import argparse
import ctypes

from wrhandle import FileWrHandle, StdoutWrHandle

VECSIZES = {
    'avx2': 32
}

def main(isa, outfile):
    # pylint: disable=missing-function-docstring
    fallback = ctypes.sizeof(ctypes.c_ulonglong)

    with FileWrHandle(outfile) if outfile is not None else StdoutWrHandle() as handle:
        handle.writeln(f'vecsize := {VECSIZES.get(isa.lower(), fallback)}')
        handle.writeln(f'ullsize := {fallback}')

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Generate processor information given SIMD ISA')
    parser.add_argument('isa', metavar='ISA', type=str, help='SIMD instruction set architecture', default=None)
    parser.add_argument('-o', '--outfile', type=str, default=None, help='Desired output file')
    main(**vars(parser.parse_args()))
