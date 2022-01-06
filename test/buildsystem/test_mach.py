''' Tests for machine-specific aspects of the build system '''

import ctypes
import platform

import pytest

from fixtures import build, rundir, root, mirror_repo, remove_mach_asm

def _is_amd64_linux():
    ''' Check if running on amd64 linux '''
    return f'{platform.system().lower()}_{platform.machine().lower()}' == 'linux_x86_64'

def _avx2_support():
    ''' Check avx2 support using /proc/cpuinfo '''
    if not _is_amd64_linux():
        return False
    with open('/proc/cpuinfo', 'r') as handle:
        while (line := handle.readline()) != '':
            if line.find('avx2'):
                return True

@pytest.mark.skipif(not _avx2_support(), reason='Unsupported arch')
def test_vecsize(build):
    ''' Verify that the expected vecsize is passed to cpp '''
    filtered = list(filter(lambda line: '-DSCC_VECSIZE=32' in line, build))
    assert filtered

@pytest.mark.skipif(not _is_amd64_linux(), reason='Unsupported arch')
def test_fallback_vecsize(remove_mach_asm, build):
    ''' Check that fallback vector size is set to sizeof(unsigned long long) '''
    filtered = list(filter(lambda line: f'-DSCC_VECSIZE={ctypes.sizeof(ctypes.c_ulonglong)}' in line, build))
    assert filtered
