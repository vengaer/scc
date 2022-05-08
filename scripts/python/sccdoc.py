#!/usr/bin/env python
''' Convert inline documentation in headers to rst source files '''

import argparse
import re

from wrhandle import FileWrHandle, StdoutWrHandle

def extract_doc_lines(file, devel):
    ''' Parse header and extract documentation lines. Always include lines starting
        with //!. If devel is True, include lines with //? too
    '''

    if devel:
        rgx = re.compile(r'//(!|\?)\s?(.*)')
    else:
        rgx = re.compile(r'//(!)\s?(.*)')
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
        doc.append(mat.group(2))

    return doc

def write_rst(doc, outfile):
    ''' Write to outfile, or stdout if no file was provided '''
    with FileWrHandle(outfile) if outfile is not None else StdoutWrHandle() as handle:
        for line in doc:
            handle.writeln(line)

def main(file, outfile, devel):
    ''' main '''
    doc = extract_doc_lines(file, devel)
    write_rst(doc, outfile)

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Convert documentation in headers to rst source')
    parser.add_argument('file', metavar='FILE', type=str, help='Path to header file')
    parser.add_argument('-o', '--outfile', type=str, default=None, help='Desired output file')
    parser.add_argument('-d', '--devel', action='store_true',
                        help='Pass to include development documentation')
    main(**vars(parser.parse_args()))
