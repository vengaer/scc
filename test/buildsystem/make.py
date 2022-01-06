''' Make command wrappers '''

import re

from subprocess import Popen, PIPE

def build(rundir, auxopts=None):
    ''' Return verbose output spat out from build system '''
    cmd = ['make', '-C', rundir, 'VERBOSE=1']
    if auxopts is not None:
        cmd.append(auxopts)
    with Popen(cmd, stdout=PIPE) as proc:
        stdout = proc.communicate()[0]
    pproced = re.sub(r'\s{2,}', ' ', stdout.decode().strip()).split('\n')
    rehandle = re.compile(r'make.*:')
    return list(filter(lambda line: rehandle.match(line) is None, pproced))
