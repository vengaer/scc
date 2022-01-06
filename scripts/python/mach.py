#!/usr/bin/env python3

''' Script generating machine information in mk format '''

import abc
import argparse
import platform

class WrHandle(metaclass=abc.ABCMeta):
    ''' Abstract write handle base class '''

    @abc.abstractmethod
    def write(self, line):
        ''' Write a line, no newline character '''

    @abc.abstractmethod
    def writeln(self, line):
        ''' Write a line, insert newline character'''

    @abc.abstractmethod
    def __enter__(self):
        ''' Should support with statement '''

    @abc.abstractmethod
    def __exit__(self, exc_type, exc_val, exc_tb):
        ''' Required cleanup '''

class FileWrHandle(WrHandle):
    ''' Write handle for file on disk '''
    def __init__(self, filename):
        # pylint: disable=consider-using-with
        self._file = open(filename, 'w', encoding='ascii')

    def close(self):
        ''' Close file handle '''
        self._file.close()

    def write(self, line):
        self._file.write(line)

    def writeln(self, line):
        self.write(f'{line}\n')

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.close()

class StdoutWrHandle(WrHandle):
    ''' Write handle for stdout '''
    def write(self, line):
        print(line, end='')

    def writeln(self, line):
        print(line)

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        pass

def main(outfile):
    # pylint: disable=missing-function-docstring

    unknown = "UNKNOWN"
    with FileWrHandle(outfile) if outfile is not None else StdoutWrHandle() as handle:
        machine = platform.machine()
        bits, linkage = platform.architecture()
        system = platform.system()
        rel = platform.release()
        ver = platform.version()

        handle.writeln(f'arch      := {machine.upper() if machine != "" else unknown}')
        handle.writeln(f'bitarch   := {bits.replace("bit","") if bits != "" else unknown}')
        handle.writeln(f'binfmt    := {linkage if linkage.upper() != "" else unknown}')
        handle.writeln(f'hostos    := {system.upper() if system != "" else unknown}')
        handle.writeln(f'hostosrel := {rel.upper() if rel != "" else unknown}')
        handle.writeln('hostosver := {}'.format(ver.upper().replace('#', '\\#') if ver != '' else unknown))

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Detect machine information')
    parser.add_argument('-o', '--outfile', type=str, default=None, help='Desired output file')
    main(**vars(parser.parse_args()))
