import pytest

def pytest_addoption(parser):
    parser.addoption('--root', action='store', default=None)

@pytest.fixture
def root(pytestconfig):
    d = pytestconfig.getoption('root')
    if d is None:
        raise RuntimeError('--root not passed on invocation')
    return d
