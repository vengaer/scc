''' Build system dependency graph tests '''

import os
import pathlib
import re

import make

from fixtures import root, rundir, mirror_repo, build

def test_header_dependencies_generated(rundir, build):
    ''' Build scc, touch a header and verify that source files are rebuilt '''
    pathlib.Path(f'{rundir}/scc/scc_svec.h').touch()
    stdout = make.build(rundir)
    for command in ['[CC] scc_svec.o', '[AR] libscc.a', '[LD] libscc.so.', '[LN] libscc.so']:
        found = list(filter(lambda line: command in line, stdout))
        assert found

def test_makefile_dependencies_generated(rundir, build):
    ''' Build scc, touch the Makefile and verify that source files are rebuilt '''
    rehandle = re.compile(r'\[CC\].*\.o')
    objs = list(filter(lambda line: rehandle.match(line) is not None, build))
    assert objs

    pathlib.Path(f'{rundir}/Makefile').touch()
    stdout = make.build(rundir)

    for obj in objs:
        assert obj in stdout

    for command in ['[AR] libscc.a', '[LD] libscc.so.', '[LN] libscc.so']:
        found = list(filter(lambda line: command in line, stdout))
        assert found

def test_mkscript_dependencies_generated(rundir, build):
    ''' Build scc, touch each of the build scripts and verify that source files are rebuilt '''
    rehandle = re.compile(r'\[CC\].*\.o')
    objs = list(filter(lambda line: rehandle.match(line) is not None, build))
    assert objs

    for ent in os.listdir(f'{rundir}/scripts/mk'):
        pathlib.Path(f'{rundir}/scripts/mk/{ent}').touch()
        stdout = make.build(rundir)

        for obj in objs:
            assert obj in stdout

        for command in ['[AR] libscc.a', '[LD] libscc.so.', '[LN] libscc.so']:
            found = list(filter(lambda line: command in line, stdout))
            assert found
