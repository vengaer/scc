''' Tests of misc. expression macros '''

import make
from fixtures import *

def test_not(script_dir):
    ''' Test logical negation '''
    eneg_ok = 'Empty negation ok'
    eneg_err = 'Empty negation failure'
    nneg_ok = 'Non-empty negation ok'
    nneg_err = 'Non-empty negation failure'
    ec, stdout, stderr = make.make_supplied([
       f'mkscripts := {script_dir}',
        'include $(mkscripts)/expr.mk',
       f'$(if $(call not,$(a)),$(info {eneg_ok}),$(error {eneg_err}))',
        'a := _',
       f'$(if $(call not,$(a)),$(error {nneg_err}),$(info {nneg_ok}))',
        '.PHONY: all',
        'all:'
    ])
    assert stderr == []
    assert ec == 0
    assert eneg_ok in stdout
    assert nneg_ok in stdout

def test_empty(script_dir):
    ''' Test empty check '''
    eneg_ok = 'Empty variable correctly reported as empty'
    eneg_err = 'Empty variable reported as non-empty'
    nneg_ok = 'Non-empty variable correctly reported as non-empty'
    nneg_err = 'Non-empty variable reported as empty'
    ec, stdout, stderr = make.make_supplied([
       f'mkscripts := {script_dir}',
        'include $(mkscripts)/expr.mk',
       f'$(if $(call empty,$(a)),$(info {eneg_ok}),$(error {eneg_err}))',
        'a := _',
       f'$(if $(call empty,$(a)),$(error {nneg_err}),$(info {nneg_ok}))',
        '.PHONY: all',
        'all:'
    ])
    assert stderr == []
    assert ec == 0
    assert eneg_ok in stdout
    assert nneg_ok in stdout

def test_require(script_dir):
    ''' Test require macro '''

    makefile = [
       f'mkscripts := {script_dir}',
        'include $(mkscripts)/expr.mk',
        '.PHONY: foo',
        'foo: $(call require,foo,BAR)'
    ]
    assert make.make_supplied(makefile, args='foo')[0] != 0
    assert make.make_supplied(makefile, args='foo BAR=asdf')[0] == 0

def test_assert(script_dir):
    ''' Test assert macro '''

    errmsg = 'VAR not set'
    makefile = [
       f'mkscripts := {script_dir}',
        'include $(mkscripts)/expr.mk',
       f'$(call assert,$(VAR),{errmsg})',
        '.PHONY: all',
        'all:'
    ]

    ec, _, stderr = make.make_supplied(makefile)
    assert ec != 0
    assert list(filter(lambda s: errmsg in s, stderr))
    ec, _, stderr = make.make_supplied(makefile, args='VAR=_')
    assert stderr == []
    assert ec == 0
    assert not list(filter(lambda s: errmsg in s, stderr))

def test_eq(script_dir):
    ''' Test eq macro '''

    ec, stdout, stderr = make.make_supplied([
       f'mkscripts := {script_dir}',
        'include $(mkscripts)/expr.mk',
        '$(call assert,$(call eq,foo,foo),foo != foo)',
        '$(call assert,$(call not,$(call eq,foo,bar)), foo == bar)',
        '$(call assert,$(call not,$(call eq,foo,foobar)), foo == foobar)',
        '$(call assert,$(call not,$(call eq,foobar,foo)), foobar == foo)',
        '$(call assert,$(call not,$(call eq,o,fo)), o == fo)',
        '.PHONY: all',
        'all:'
    ])

    assert stderr == []
    assert ec == 0

def test_diff(script_dir):
    ''' Test diff macro '''

    ec, stdout, stderr = make.make_supplied([
       f'mkscripts := {script_dir}',
        'include $(mkscripts)/stack.mk',
        '$(info $(call diff,a b c,a c))',
        '$(info $(call diff,-std=c11 -Wall -Wextra -Wpedantic,-std=c11 -Wall))',
        '$(info $(call diff,-std=c99 -Wall -Wunused,-std=c11))',
        '$(info $(call diff,quack,ack))',
        '.PHONY: all',
        'all:'
    ])
    assert stdout[0] == 'b'
    assert stdout[1] == '-Wextra -Wpedantic'
    assert stdout[2] == '-std=c99 -Wall -Wunused'
    assert stdout[3] == 'quack'
    assert stderr == []
    assert ec == 0
