''' Build system dependency graph tests '''

import pathlib

import make

from fixtures import root, rundir, mirror_repo, build

def test_header_dependencies_generated(rundir, build):
    ''' Build scc, touch a header and verify that source files are rebuilt '''
    pathlib.Path(f'{rundir}/scc/scc_svec.h').touch()
    stdout = make.build(rundir, auxopts='-j8')
    for command in ['[CC] scc_svec.o', '[AR] libscc.a', '[LD] libscc.so.', '[LN] libscc.so']:
        found = list(filter(lambda line: command in line, stdout))
        assert found
