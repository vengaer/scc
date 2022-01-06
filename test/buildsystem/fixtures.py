''' Build system fixtures '''

import os
import re
import shutil

from subprocess import Popen, PIPE

import pytest

@pytest.fixture
def root(pytestconfig):
    ''' Get root directory from command line options '''
    rootdir = pytestconfig.getoption('root')
    if rootdir is None:
        raise RuntimeError('--root not passed on invocation')
    return rootdir

@pytest.fixture
def rundir(root):
    return f'{root}/build/buildsystem'

@pytest.fixture(autouse=True)
def mirror_repo(root, rundir):
    ''' Mirror relevant parts of the repository top level '''

    builddir = os.path.join(root, 'build')

    if not os.path.exists(builddir):
        os.mkdir(builddir)
    elif os.path.exists(rundir):
        shutil.rmtree(rundir)

    os.mkdir(rundir)
    exclude = ['test', 'build', '.git', 'tags']

    for ent in list(filter(lambda e: e not in exclude, os.listdir(root))):
        src = os.path.join(root, ent)
        dst = os.path.join(rundir, ent)
        if os.path.isdir(src):
            shutil.copytree(src, dst, False, None)
        else:
            shutil.copyfile(src, dst)
    try:
        yield
    finally:
        shutil.rmtree(rundir)

@pytest.fixture
def build(rundir):
    ''' Return verbose output spat out from build system '''
    with Popen(['make', '-C', rundir, 'VERBOSE=1'], stdout=PIPE) as proc:
        stdout = proc.communicate()[0]
    pproced = re.sub(r'\s{2,}', ' ', stdout.decode().strip()).split('\n')
    rehandle = re.compile(r'make.*:')
    return list(filter(lambda line: rehandle.match(line) is None, pproced))

@pytest.fixture
def remove_mach_asm(rundir):
    ''' Remove asm tree in {rundir}/mach '''
    shutil.rmtree(f'{rundir}/mach/asm')
