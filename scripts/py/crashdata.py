#!/usr/bin/env python3
''' Read the crash file and dump it as a formatted array of bytes '''

import argparse

from wrhandle import FileWrHandle, StdoutWrHandle

_LINE_LIM = 8

def read_and_dump(file, outfile, skip):
    ''' Read the file and dump its contents '''
    with open(file, 'rb') as handle:
        data = handle.read()[skip:]

    with FileWrHandle(outfile) if outfile is not None else StdoutWrHandle() as handle:
        handle.write(f'unsigned char data[] = {{\n{4 * " "}')
        for i, byte in enumerate(data):
            handle.write(f'0x{byte:02x}')
            if i < len(data) - 1:
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
    read_and_dump(**vars(parser.parse_args()))
