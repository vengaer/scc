''' Build system volatile variable tests '''

import make

from fixtures import project_root, script_dir

def test_volatile_variable_restoration(script_dir):
    ''' Repeatedly push and pop volatile variables '''
    ec, stdout, stderr = make.make_supplied([
       f'mkscripts := {script_dir}',
        'CFLAGS    := -Wall -Wextra -Wpedantic',
        'LDLIBS    := -lrt -lpthread',
        'include $(mkscripts)/volatile.mk',
        '$(call volatile-push)',
        'CFLAGS    += -Wunused',
        '$(call volatile-push)',
        'CFLAGS    += -Wunknown-pragmas',
        '$(call volatile-push)',
        'LDLIBS    += -lasan',
        '$(call volatile-push)',
        '$(call volatile-push)',
        '$(info $(CFLAGS))',
        '$(info $(LDLIBS))',
        '$(call volatile-pop)',
        '$(info $(CFLAGS))',
        '$(info $(LDLIBS))',
        '$(call volatile-pop)',
        '$(info $(CFLAGS))',
        '$(info $(LDLIBS))',
        '$(call volatile-pop)',
        '$(info $(CFLAGS))',
        '$(info $(LDLIBS))',
        '$(call volatile-pop)',
        '$(info $(CFLAGS))',
        '$(info $(LDLIBS))',
        '$(call volatile-pop)',
        '$(info $(CFLAGS))',
        '$(info $(LDLIBS))',
        '.PHONY: all',
        'all:'
    ])
    assert stderr == []
    assert ec == 0
    assert stdout[0] == '-Wall -Wextra -Wpedantic -Wunused -Wunknown-pragmas'
    assert stdout[1] == '-lrt -lpthread -lasan'
    assert stdout[2] == '-Wall -Wextra -Wpedantic -Wunused -Wunknown-pragmas'
    assert stdout[3] == '-lrt -lpthread -lasan'
    assert stdout[4] == '-Wall -Wextra -Wpedantic -Wunused -Wunknown-pragmas'
    assert stdout[5] == '-lrt -lpthread -lasan'
    assert stdout[6] == '-Wall -Wextra -Wpedantic -Wunused -Wunknown-pragmas'
    assert stdout[7] == '-lrt -lpthread'
    assert stdout[8] == '-Wall -Wextra -Wpedantic -Wunused'
    assert stdout[9] == '-lrt -lpthread'
    assert stdout[10] == '-Wall -Wextra -Wpedantic'
    assert stdout[11] == '-lrt -lpthread'
