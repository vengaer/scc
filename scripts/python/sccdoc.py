#!/usr/bin/env python
''' Convert inline documentation in headers to rst source files '''

import argparse
import re

from wrhandle import FileWrHandle, StdoutWrHandle

def extract_doc_lines(file):
    ''' Parse header and extract lines starting with //! '''
    rgx = re.compile(r'//!\s?(.*)')
    with open(file, 'r', encoding='ascii') as handle:
        content = handle.readlines()

    in_block = False
    doc = []
    for line in content:
        if not (mat := rgx.match(line)):
            if in_block:
                doc.append('')
            in_block = False
            continue
        in_block = True
        doc.append(mat.group(1))

    return doc

def write_rst(doc, outfile):
    ''' Write to outfile, or stdout if no file was provided '''
    with FileWrHandle(outfile) if outfile is not None else StdoutWrHandle() as handle:
        for line in doc:
            handle.writeln(line)

def main(file, outfile):
    ''' main '''
    doc = extract_doc_lines(file)
    write_rst(doc, outfile)

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Convert documentation in headers to rst source')
    parser.add_argument('file', metavar='FILE', type=str, help='Path to header file')
    parser.add_argument('-o', '--outfile', type=str, default=None, help='Desired output file')
    main(**vars(parser.parse_args()))
