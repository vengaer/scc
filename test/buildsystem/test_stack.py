''' Build system stack tests '''

import make

from fixtures import *

def test_stack_empty_on_init(script_dir):
    ''' Initialize a stack and verify that stack-empty returns true '''
    ok_str = 'Initialized stack is empty'
    err_str = 'Initialized stack is not empty'
    ec, stdout, stderr = make.make_supplied([
       f'mkscripts := {script_dir}',
        'include $(mkscripts)/stack.mk',
        '$(call stack-init,stack)',
       f'$(if $(call stack-empty,stack),$(info {ok_str}),$(error {err_str}))',
        '.PHONY: all',
        'all:'
    ])
    assert ec == 0
    assert ok_str in stdout
    assert err_str not in stderr

def test_stack_non_empty_after_push(script_dir):
    ''' Initialize and push to stack, verify that stack-empty returns false '''
    ok_str = 'Stack is non-empty'
    err_str = 'Stack is empty'
    ec, stdout, stderr = make.make_supplied([
       f'mkscripts := {script_dir}',
        'include $(mkscripts)/stack.mk',
        '$(call stack-init,stack)',
        '$(call stack-push,stack,asdf)',
       f'$(if $(call stack-empty,stack),$(error {err_str}),$(info {ok_str}))',
        '.PHONY: all',
        'all:'
    ])
    assert ec == 0
    assert ok_str in stdout
    assert err_str not in stderr

def test_stack_handles_spaces(script_dir):
    ''' Initialize and push several strings with spaces and verify that they remain correct '''
    strings = [
        'first string',
        'second string',
        'third string with a few more words'
    ]

    ec, stdout, stderr = make.make_supplied([
       f'mkscripts := {script_dir}',
        'include $(mkscripts)/stack.mk',
        '$(call stack-init,stack)',
       f'$(call stack-push,stack,{strings[0]})',
        '$(call assert,$(call not,$(call stack-empty,stack)),Empty after first push)',
        '$(info $(call stack-top,stack))',
       f'$(call stack-push,stack,{strings[1]})',
        '$(call assert,$(call not,$(call stack-empty,stack)),Empty after second push)',
        '$(info $(call stack-top,stack))',
       f'$(call stack-push,stack,{strings[2]})',
        '$(call assert,$(call not,$(call stack-empty,stack)),Empty after third push)',
        '$(info $(call stack-top,stack))',
        '$(call stack-pop,stack)',
        '$(call assert,$(call not,$(call stack-empty,stack)),Empty after first pop)',
        '$(info $(call stack-top,stack))',
        '$(call stack-pop,stack)',
        '$(call assert,$(call not,$(call stack-empty,stack)),Empty after second pop)',
        '$(info $(call stack-top,stack))',
        '$(call stack-pop,stack)',
        '$(call assert,$(call stack-empty,stack),Non-empty after third pop)',
        '.PHONY: all',
        'all:'
    ])
    assert ec == 0
    assert not list(filter(lambda s: s, stderr))

    for i in reversed(range(len(strings) - 1)):
        strings.append(strings[i])

    for exp, res in zip(strings, stdout):
        assert exp == res

def test_stack_top_on_empty_disallowed(script_dir):
    ''' Try to call stack-top on empty stack and verify that it calls $(error ...) '''

    ec, _, stderr = make.make_supplied([
       f'mkscripts := {script_dir}',
        'include $(mkscripts)/stack.mk',
        '$(call stack-init,stack)',
        '$(call stack-top,stack)'
    ])

    assert ec != 0
    assert list(filter(lambda s: 'stack is empty' in s.lower(), stderr))

def test_stack_pop_on_empty_disallowed(script_dir):
    ''' try to call stack-pop on empty stack and verify that it calls $(error ...) '''

    ec, _, stderr = make.make_supplied([
       f'mkscripts := {script_dir}',
        'include $(mkscripts)/stack.mk',
        '$(call stack-init,stack)',
        '$(call stack-pop,stack)'
    ])

    assert ec != 0
    assert list(filter(lambda s: 'stack is empty' in s.lower(), stderr))

def test_stack_handles_intermittently_empty_slots(script_dir):
    ''' Push an empty string to the stack and verify that it's still empty '''

    ec, stdout, stderr = make.make_supplied([
       f'mkscripts := {script_dir}',
        'include $(mkscripts)/stack.mk',
        '$(call stack-init,stack)',
        '$(call stack-push,stack,a)',
        '$(call stack-push,stack)',
        '$(call stack-push,stack,b)',
        '$(call assert,$(call eq,$(call stack-top,stack),b),top not b)',
        '$(call stack-pop,stack)',
        '$(call assert,$(call not,$(call stack-top,stack)),top not empty)',
        '$(call stack-pop,stack)',
        '$(call assert,$(call eq,$(call stack-top,stack),a),top not a)',
        '$(call stack-pop,stack)',
        '$(call assert,$(call stack-empty,stack),stack not empty)'
        '.PHONY: all',
        'all:'
    ])

    assert stderr == []
    assert ec == 0
    assert not list(filter(lambda s: s, stderr))

def test_stack_value_init(script_dir):
    ''' Initialize a stack with a value '''

    ec, stdout, stderr = make.make_supplied([
       f'mkscripts := {script_dir}',
        'include $(mkscripts)/stack.mk',
        '$(call stack-init,stack,a b c)',
        '$(call assert,$(call eq,$(call stack-top,stack),a b c))',
        '$(info $(stack))',
        '.PHONY: all',
        'all:'
    ])
    assert stderr == []
    assert ec == 0

def test_global_stack(script_dir):
    ''' Verify the behavior of pushing and poping to the global stack '''

    ec, stdout, stderr = make.make_supplied([
       f'mkscripts := {script_dir}',
        'include $(mkscripts)/stack.mk',
        'CFLAGS   := -Wall -Wextra -Wpedantic',
        'CPPFLAGS := -DFOO -DBAR',
        '$(call push,CFLAGS)',
        '$(call push,CPPFLAGS)',
        'CFLAGS   += -Wshadow',
        'CPPFLAGS += -DNDEBUG',
        '$(info $(CFLAGS))',
        '$(info $(CPPFLAGS))',
        '$(call pop,CPPFLAGS)'
        '$(call pop,CFLAGS)',
        '$(info $(CFLAGS))',
        '$(info $(CPPFLAGS))',
        '.PHONY: all',
        'all:'
    ])

    assert stderr == []
    assert ec == 0
    assert stdout[0] == '-Wall -Wextra -Wpedantic -Wshadow'
    assert stdout[1] == '-DFOO -DBAR -DNDEBUG'
    assert stdout[2] == '-Wall -Wextra -Wpedantic'
    assert stdout[3] == '-DFOO -DBAR'
