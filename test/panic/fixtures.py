import os
import pytest
import subprocess

@pytest.fixture
def builddir(pytestconfig):
    d = pytestconfig.getoption('builddir')
    if d is None:
        raise RuntimeError('--builddir not passed on invocation')
    return d

@pytest.fixture
def proc(builddir, request):
    return subprocess.Popen([os.path.join(builddir, request.node.name)],
                             stdin=subprocess.PIPE,
                             stdout=subprocess.PIPE,
                             stderr=subprocess.PIPE)

@pytest.fixture
def nullproc(proc):
    proc.wait()
    return proc.returncode

