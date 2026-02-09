#!/usr/bin/env python3

''' Script generating machine information in mk format '''

import argparse
import contextlib
import multiprocessing
import os
import platform
import subprocess
import tempfile

from wrhandle import FileWrHandle, StdoutWrHandle

def check_fixed_width_integer(compiler, integer):
    code = '\n'.join([
        "#include <stdint.h>",
        "#include <stdio.h>",
        "",
        "int main(void) {",
        f"    printf(\"%zu\\n\", sizeof({integer}));",
        "}"
    ])

    fd, path = tempfile.mkstemp(suffix='.c')
    os.close(fd)
    try:
        with open(path, "w", encoding="ascii") as handle:
            handle.write(code)

        with contextlib.suppress(subprocess.CalledProcessError):
            subprocess.check_output([compiler, '-std=c99', f'-o{os.devnull}', path], stderr=subprocess.DEVNULL)
            return "y"

    finally:
        if os.path.exists(path):
            os.unlink(path)

    return ""

def main(outfile, compiler):
    # pylint: disable=missing-function-docstring

    unknown = "UNKNOWN"
    abis = {
        'linux_x86_64': 'SYSV',
        'freebsd_x86_64': 'SYSV',
        'darwin_x86_64': 'SYSV',
        'solaris_x86_64': 'SYSV'
    }

    with FileWrHandle(outfile) if outfile is not None else StdoutWrHandle() as handle:
        machine = platform.machine()
        bits, linkage = platform.architecture()
        system = platform.system()
        rel = platform.release()
        ver = platform.version().replace('#', '\\#')
        abi = abis.get(f'{system.lower()}_{machine.lower()}', unknown)
        try:
            cores = multiprocessing.cpu_count()
        except NotImplementedError:
            cores = 1

        handle.writeln(f'arch          := {machine.upper() if machine != "" else unknown}')
        handle.writeln(f'bitarch       := {bits.replace("bit","") if bits != "" else unknown}')
        handle.writeln(f'binfmt        := {linkage if linkage.upper() != "" else unknown}')
        handle.writeln(f'hostos        := {system.upper() if system != "" else unknown}')
        handle.writeln(f'hostosrel     := {rel.upper() if rel != "" else unknown}')
        handle.writeln(f'hostosver     := {ver.upper() if ver != "" else unknown}')
        handle.writeln(f'abi           := {abi}')
        handle.writeln(f'cpu_cores     := {cores}')
        handle.writeln(f'arch_lower    := {machine.lower() if machine != "" else unknown.lower()}')
        handle.writeln(f'abi_lower     := {abi.lower()}')
        handle.writeln(f'u32_available := {check_fixed_width_integer(compiler, "uint32_t")}')
        handle.writeln(f'u64_available := {check_fixed_width_integer(compiler, "uint64_t")}')

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Detect machine information')
    parser.add_argument('-o', '--outfile', type=str, default=None, help='Desired output file')
    parser.add_argument('compiler', metavar='COMPILER', help='Name of the compiler to use')
    main(**vars(parser.parse_args()))
