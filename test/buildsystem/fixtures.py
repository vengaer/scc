import os
import sys
import pytest

@pytest.fixture(autouse=True, scope='session')
def clear_environment():
    try:
        del os.environ['MAKEFLAGS']
    except KeyError:
        pass

@pytest.fixture
def project_root(pytestconfig):
    root = pytestconfig.getoption('project_root')
    if root is None:
        raise RuntimeError('--project-root not passed on invocation')
    return root

@pytest.fixture
def script_dir(project_root):
    return os.path.join(project_root, 'scripts/mk')

@pytest.fixture
def build_dir(project_root):
    return os.path.join(project_root, 'build/test/buildsystem')

@pytest.fixture
def pyscripts_in_path(project_root):
    sys.path.append(f'{project_root}/scripts/python')
