''' Tests for path macros '''

import os

import make
from fixtures import *

@pytest.mark.parametrize('abspath, start', [
    ('/home/vilhelm/repos/c/scc/docs', '/home/vilhelm/repos/c/scc/build/docs'),
    ('/home/vilhelm/repos/c/scc/test/fuzz/hashmap', '/home/vilhelm/repos/c/scc/build/test/fuzz/hashmap'),
    ('/home/vilhelm/repos/c/scc/test/fuzz/corpora', '/root')
])
def test_path_relative_to(abspath, start, script_dir):
    ''' Test relative path derivation '''

    ec, stdout, stderr = make.make_supplied([
       f'mkscripts := {script_dir}',
        'include $(mkscripts)/path.mk',
       f'$(info $(call path-relative-to,{abspath},{start}))',
        '.PHONY: all',
        'all:'
    ])
    assert not stderr
    assert ec == 0
    relpath = os.path.relpath(start, abspath)
    assert stdout[0] == os.path.join(abspath, relpath)
