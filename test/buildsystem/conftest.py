def pytest_addoption(parser):
    parser.addoption('--project-root', action='store', default=None)
