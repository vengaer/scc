import os
import pytest

@pytest.fixture
def project_root(pytestconfig):
    root = pytestconfig.getoption('project_root')
    if root is None:
        raise RuntimeError('--project-root not passed on invocation')
    return root

@pytest.fixture
def script_dir(project_root):
    return os.path.join(project_root, 'scripts/mk')
