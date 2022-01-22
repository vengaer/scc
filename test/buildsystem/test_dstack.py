''' Build system diff stack tests '''

import make

from fixtures import project_root, script_dir

_JOIN_SYM = '<<>>'

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
    assert len(stdout[3].strip().split(_JOIN_SYM)) == 1
    assert len(stdout[4].strip().split(_JOIN_SYM)) == 2

def test_dstack_sub(script_dir):
    ''' Test dstack top subtraction '''
    ec, stdout, stderr = make.make_supplied([
       f'mkscripts := {script_dir}',
        'include $(mkscripts)/stack.mk',
        '$(call dstack-init,dstack,-Wall -Wextra -Wpedantic -Wunused -Wunknown-pragmas)',
        '$(info $(call dstack-top,dstack))',
        '$(call dstack-sub,dstack,-Wextra -Wpedantic -Wunused)',
        '$(info $(call dstack-top,dstack))',
        '$(call dstack-dump,dstack)',
        '.PHONY: all',
        'all:'
    ])

    assert stderr == []
    assert ec == 0
    assert stdout[0] == '-Wall -Wextra -Wpedantic -Wunused -Wunknown-pragmas'
    assert stdout[1] == '-Wall -Wunknown-pragmas'
    assert len(stdout[2].strip().split(_JOIN_SYM)) == 3
