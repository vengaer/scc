import os
import pytest
import re
import subprocess

@pytest.fixture
def builddir(pytestconfig):
    d = pytestconfig.getoption('builddir')
    if d is None:
        raise RuntimeError('--builddir not passed on invocation')
    return d

@pytest.fixture
def proc(builddir, request):
    return subprocess.Popen([os.path.join(builddir, re.match('test_([^[]+)', request.node.name).groups(1)[0])],
                             stdin=subprocess.PIPE,
                             stdout=subprocess.PIPE,
                             stderr=subprocess.PIPE)

@pytest.fixture
def nullproc(proc):
    proc.wait()
    return proc.returncode

@pytest.fixture
def uniproc(proc):
    proc.wait()
    return proc.returncode, *map(lambda s: s.decode('utf-8').strip(), proc.communicate())

@pytest.fixture
def biproc(proc, request):
    out, err = map(lambda s: s.decode('utf-8').strip(), proc.communicate(input=request.param.encode('utf-8')))
    proc.wait()
    return proc.returncode, out, err

