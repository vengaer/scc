#!/usr/bin/env python3

import argparse

from wrhandle import FileWrHandle, StdoutWrHandle

_LINE_LIM = 8

def main(file, outfile):
    with open(file, 'rb') as handle:
        data = handle.read()

    with FileWrHandle(outfile) if outfile is not None else StdoutWrHandle() as handle:
        handle.write('unsigned char data[] = {\n\t')
        for i, byte in enumerate(data):
            handle.write('0x{:02x}'.format(byte))
            if i < len(data) - 1:
                handle.write(',')
            handle.write(' ')
            if not ((i + 1) % _LINE_LIM):
                handle.write('\n\t')

        handle.writeln('\n}')

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Convert fuzzer crash file to C array')
    parser.add_argument('file', metavar='FILE', type=str, help='Path to the crash file')
    parser.add_argument('-o', '--outfile', action='store', default=None,
                        help='Path to write the generated array to')
    main(**vars(parser.parse_args()))
