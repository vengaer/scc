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

def test_dstack_pop(script_dir):
    ''' Repeatedly call dstack-add and dstack-sub and verify the results '''
    ec, stdout, stderr = make.make_supplied([
       f'mkscripts := {script_dir}',
        'include $(mkscripts)/stack.mk',
        '$(call dstack-init,dstack,-Wall -Wextra)',
        '$(call dstack-add,dstack,-Wpedantic)',
        '$(call dstack-add,dstack,-Wunused -Wunknown-pragmas)',
        '$(call dstack-sub,dstack,-Wall -Wextra)',
        '$(call dstack-add,dstack,-Wall)',
        '$(call dstack-sub,dstack,-Wall)',
        '$(call dstack-add,dstack,-Wall -Wunused)',
        '$(call dstack-sub,dstack,-Wall -Wunused)',
        '$(info $(call dstack-top,dstack))',
        '$(call dstack-pop,dstack)',
        '$(info $(call dstack-top,dstack))',
        '$(call dstack-pop,dstack)',
        '$(info $(call dstack-top,dstack))',
        '$(call dstack-pop,dstack)',
        '$(info $(call dstack-top,dstack))',
        '$(call dstack-pop,dstack)',
        '$(info $(call dstack-top,dstack))',
        '$(call dstack-pop,dstack)',
        '$(info $(call dstack-top,dstack))',
        '$(call dstack-pop,dstack)',
        '$(info $(call dstack-top,dstack))',
        '$(call dstack-pop,dstack)',
        '$(info $(call dstack-top,dstack))',
        '.PHONY: all',
        'all:'
    ])
    assert stderr == []
    assert ec == 0
    assert stdout[0] == '-Wpedantic -Wunknown-pragmas'
    assert stdout[1] == '-Wpedantic -Wunknown-pragmas -Wall -Wunused'
    assert stdout[2] == '-Wpedantic -Wunknown-pragmas -Wunused'
    assert stdout[3] == '-Wpedantic -Wunknown-pragmas -Wunused -Wall'
    assert stdout[4] == '-Wpedantic -Wunknown-pragmas -Wunused'
    assert stdout[5] == '-Wpedantic -Wunknown-pragmas -Wunused -Wall -Wextra'
    assert stdout[6] == '-Wpedantic -Wall -Wextra'
    assert stdout[7] == '-Wall -Wextra'

def test_dstack_push(script_dir):
    ''' Repeatedly push values to a diff stack and verify the results '''
    ec, stdout, stderr = make.make_supplied([
       f'mkscripts := {script_dir}',
        'include $(mkscripts)/stack.mk',
        '$(call dstack-init,dstack,-Wall -Wextra)',
        '$(call dstack-push,dstack,-Wpedantic)',
        '$(info $(call dstack-top,dstack))',
        '$(call dstack-pop,dstack)',
        '$(info $(call dstack-top,dstack))',
        '$(call dstack-push,dstack,-Wall -Wextra -Wpedantic -Wunused)',
        '$(info $(call dstack-top,dstack))'
        '$(call dstack-push,dstack,$(call dstack-top,dstack))'
        '$(info $(call dstack-top,dstack))',
        '$(call dstack-pop,dstack)',
        '$(info $(call dstack-top,dstack))',
        '$(call dstack-pop,dstack)',
        '$(info $(call dstack-top,dstack))',
        '$(call dstack-push,dstack)'
        '$(info $(call dstack-top,dstack))',
        '.PHONY: all',
        'all:'
    ])
    assert stderr == []
    assert ec == 0
    assert stdout[0] == '-Wpedantic'
    assert stdout[1] == '-Wall -Wextra'
    assert stdout[2] == '-Wall -Wextra -Wpedantic -Wunused'
    assert stdout[3] == '-Wall -Wextra -Wpedantic -Wunused'
    assert stdout[4] == '-Wall -Wextra -Wpedantic -Wunused'
    assert stdout[5] == '-Wall -Wextra'
    assert stdout[6] == '-Wall -Wextra'
