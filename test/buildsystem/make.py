import subprocess
import tempfile

def make(path, args=None):
    cmd = ['make', '-f', path]
    if args is not None:
        cmd += args.split()
    ''' Run make on the makefile at path. Return exit status, stdout and stderr '''
    proc = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    decode_stream = lambda b: b.decode('ascii').strip().split('\n')
    return proc.returncode, decode_stream(proc.stdout), decode_stream(proc.stderr)

def make_supplied(contents, args=None, directory=None):
    ''' Create a temporary file, write contents to it and run make on it '''
    with tempfile.NamedTemporaryFile(mode='w', dir=directory, encoding='ascii', suffix='.mk') as handle:
        handle.file.write('\n'.join(contents))
        handle.file.write('\n')
        handle.file.flush()
        return make(handle.name, args)
