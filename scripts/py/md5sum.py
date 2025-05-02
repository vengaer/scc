#!/usr/bin/env python

import argparse
import asyncio
import hashlib
import os

_formats = {
    'default': lambda digest, file: f'{digest}  {file}',
    'make': lambda digest, file: f'{os.path.basename(file).replace(".", "")}_md5 := {digest}'
}

async def md5sum(mode, file, async_print, fmt):
    md5 = hashlib.md5()
    def update(chunk):
        md5.update(chunk if isinstance(chunk, bytes) else chunk.encode())
    with open(file, mode) as handle:
        while chunk := handle.read(8192):
            update(chunk)

    digest = md5.hexdigest()
    if async_print:
        print(_formats[fmt](digest=digest, file=file))

    return file, digest

async def calc_and_print(binary, files, outfile, fmt):
    mode = 'rb' if binary else 'r'
    digests = await asyncio.gather(*(md5sum(mode, file, outfile is None, fmt) for file in files))

    if outfile is not None:
        with open(outfile, 'w') as handle:
            for file, digest in digests:
                handle.write(_formats[fmt](digest=digest, file=file))
                handle.write('\n')

async def main():
    parser = argparse.ArgumentParser('Compute md5sum of file')
    parser.add_argument('-b', '--binary', help='Read in binary mode', action='store_true')
    parser.add_argument('-o', '--outfile', help='File to write to, default is stdout', default=None)
    parser.add_argument('-f', '--fmt', help='Format to write output in',
                        choices=('default', 'make'), default='default')
    parser.add_argument('files', metavar='FILE', nargs='+')
    await calc_and_print(**vars(parser.parse_args()))

if __name__ == '__main__':
    asyncio.run(main())
