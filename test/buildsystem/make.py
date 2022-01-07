''' Make command wrappers '''

import multiprocessing
import re

from subprocess import Popen, PIPE

def build(rundir, jobs=None):
    ''' Return verbose output spat out from build system '''
    try:
        jobs = jobs if jobs is not None else multiprocessing.cpu_count()
    except NotImplementedError:
        jobs = 1
    cmd = ['make', '-C', rundir, 'VERBOSE=1', f'-j{jobs}']
    with Popen(cmd, stdout=PIPE) as proc:
        stdout = proc.communicate()[0]
    pproced = re.sub(r'\s{2,}', ' ', stdout.decode().strip()).split('\n')
    rehandle = re.compile(r'make.*:')
    return list(filter(lambda line: rehandle.match(line) is None, pproced))
