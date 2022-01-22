''' Build system diff stack tests '''

import make

from fixtures import project_root, script_dir

def test_dstack_add(script_dir):
    ''' Test dstack top addition '''
    ec, stdout, stderr = make.make_supplied([
       f'mkscripts := {script_dir}',
        'include $(mkscripts)/stack.mk',
        '$(call dstack-init,dstack,-Wall -Wextra)',
        '$(info $(call dstack-top,dstack))',
        '$(call dstack-add,dstack,-Wpedantic)',
        '$(info $(call dstack-top,dstack))'
        '$(call dstack-add,dstack,-Wpedantic)',
        '$(info $(call dstack-top,dstack))',
        '$(call dstack-dump,dstack)',
        '$(call dstack-add,dstack,-Wunused -Wunknown-pragmas)',
        '$(call dstack-dump,dstack)',
        '.PHONY: all',
        'all:'
    ])
    assert stderr == []
    assert ec == 0
    assert stdout[0] == '-Wall -Wextra'
    assert stdout[1] == '-Wall -Wextra -Wpedantic'
    assert stdout[2] == '-Wall -Wextra -Wpedantic'
    assert len(stdout[3].strip().split('<<>>')) == 1
    assert len(stdout[4].strip().split('<<>>')) == 2

