import subprocess
import tempfile

def make(path):
    ''' Run make on the makefile at path. Return exit status, stdout and stderr '''
    proc = subprocess.run(['make', '-f', path],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    decode_stream = lambda b: b.decode('ascii').strip().split('\n')
    return proc.returncode, decode_stream(proc.stdout), decode_stream(proc.stderr)

def make_supplied(contents):
    ''' Create a temporary file, write contents to it and run make on it '''
    with tempfile.NamedTemporaryFile(mode='w', encoding='ascii', suffix='.mk') as handle:
        handle.file.write('\n'.join(contents))
        handle.file.write('\n')
        handle.file.flush()
        return make(handle.name)
