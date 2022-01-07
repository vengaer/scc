#!/usr/bin/env python3

''' Script generating machine information in mk format '''

import argparse
import multiprocessing
import platform

from wrhandle import FileWrHandle, StdoutWrHandle

def main(outfile):
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

        handle.writeln(f'arch       := {machine.upper() if machine != "" else unknown}')
        handle.writeln(f'bitarch    := {bits.replace("bit","") if bits != "" else unknown}')
        handle.writeln(f'binfmt     := {linkage if linkage.upper() != "" else unknown}')
        handle.writeln(f'hostos     := {system.upper() if system != "" else unknown}')
        handle.writeln(f'hostosrel  := {rel.upper() if rel != "" else unknown}')
        handle.writeln(f'hostosver  := {ver.upper() if ver != "" else unknown}')
        handle.writeln(f'abi        := {abi}')
        handle.writeln(f'cpu_cores  := {cores}')
        handle.writeln(f'arch_lower := {machine.lower() if machine != "" else unknown.lower()}')
        handle.writeln(f'abi_lower  := {abi.lower()}')

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Detect machine information')
    parser.add_argument('-o', '--outfile', type=str, default=None, help='Desired output file')
    main(**vars(parser.parse_args()))
