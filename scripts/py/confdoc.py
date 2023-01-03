#!/usr/bin/env python

''' Generated rst documentation for conftool specification file '''

import argparse
import json

from wrhandle import FileWrHandle, StdoutWrHandle

def translate(spec):
    ''' Translate json specification to legible rst '''

    content = []
    for ent in spec['entries']:
        name = ent['name']
        etype = ent['entrytype']
        default = str(ent['default']).strip()
        if not default:
            default = 'None'

        deps = ent['depends']
        if not deps:
            deps = 'None'
        else:
            deps = ', '.join(deps)

        rst = [ \
           f'.. _{name.lower()}:',
           f'.. option:: {name}',
            '',
           f'    :type: {etype}',
           f'    :default: {default}',
           f'    :depends: {deps}',
            '',
           f'   {ent["help"]}',
            ''
        ]

        content += rst

    return content

def main(specification, outfile):
    # pylint: disable=missing-function-docstring

    with open(specification, 'r', encoding='ascii') as handle:
        spec = json.load(handle)

    output = translate(spec)

    with FileWrHandle(outfile) if outfile is not None else StdoutWrHandle() as handle:
        for line in output:
            handle.writeln(line)

if __name__ == '__main__':
    parser = argparse.ArgumentParser(
            description='Generate rst documentation form conftool specification file')
    parser.add_argument('-s', '--specification', action='store', default='.conftool.json',
        help='Path to conftool specification')
    parser.add_argument('-o', '--outfile', action='store', default=None,
        help='Path to write the generated rst document to')
    main(**vars(parser.parse_args()))
