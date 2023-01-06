#!/usr/bin/env python

import argparse
import asyncio

import aiofiles

from wrhandle import FileWrHandle, StdoutWrHandle

async def aread(index, file):
    async with aiofiles.open(file, 'r') as handle:
        return index, await handle.read()

async def cat(files, outfile):
    files = {f for f in files}

    raw = await asyncio.gather(*(aread(i, f) for i, f in enumerate(files)))
    raw = sorted(raw, key=lambda t: t[0])

    with StdoutWrHandle() if outfile is None else FileWrHandle(outfile) as handle:
        for _, f in raw:
            handle.writeln(f)

async def main():
    parser = argparse.ArgumentParser('Concatenate files')
    parser.add_argument('files', metavar='FILES', nargs='+', help='Files to concatenate')
    parser.add_argument('-o', '--outfile', help='File to write to', default=None)
    await cat(**vars(parser.parse_args()))

if __name__ == '__main__':
    asyncio.run(main())
