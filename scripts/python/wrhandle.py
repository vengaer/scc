''' Utilities for unified stream and file handling '''

import abc

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
