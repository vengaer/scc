#!/usr/bin/env python3
''' Read the crash file and dump it as a formatted array of bytes '''

import argparse
import functools
import os
import sys

from wrhandle import FileWrHandle, StdoutWrHandle

_LINE_LIM = 8

_TYPES = {
    1: 'uint8_t',
    2: 'uint16_t',
    4: 'uint32_t',
    8: 'uint64_t'
}

def read_and_dump(file, outfile, skip, group):
    ''' Read the file and dump its contents '''
    with open(file, 'rb') as handle:
        data = handle.read()[skip:]

    fmt = f'0x{{:0{group}x}}'
    with FileWrHandle(outfile) if outfile is not None else StdoutWrHandle() as handle:
        handle.write(f'{_TYPES[int(group)]} data[] = {{\n{4 * " "}')
        with open(file, 'rb') as infile:
            read_chunk = functools.partial(infile.read, int(group))
            sz = os.fstat(infile.fileno()).st_size

            for i, chunk in enumerate(iter(read_chunk, b'')):
                val = int.from_bytes(chunk, sys.byteorder)
                handle.write(fmt.format(val))
                if i < sz // int(group) - 1:
                    handle.write(',')
                handle.write(' ')
                if not (i + 1) % _LINE_LIM:
                    handle.write(f'\n{4 * " "}')
        handle.writeln('\n}')

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Convert fuzzer crash file to C array')
    parser.add_argument('file', metavar='FILE', type=str, help='Path to the crash file')
    parser.add_argument('-o', '--outfile', action='store', default=None,
                        help='Path to write the generated array to')
    parser.add_argument('-s', '--skip', action='store', type=int, default=0,
                        help='Number of initial bytes to discard')
    parser.add_argument('-g', '--group', default=1, help='Size of each input variable')
    read_and_dump(**vars(parser.parse_args()))
