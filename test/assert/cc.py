import os
import subprocess
import tempfile

def compile_file(file, cc=None):
    cc = cc if cc is not None else 'clang'
    proc = subprocess.run([f'cc', '-o', f'{file}.bin', file], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    decode = lambda b: list(filter(lambda s: s, b.decode('utf-8').strip().split('\n')))
    return proc.returncode, decode(proc.stdout), decode(proc.stderr)


def compile_supplied(contents, cc=None):
    cc = cc if cc is not None else 'clang'

    with tempfile.NamedTemporaryFile(mode='w', dir=None, encoding='ascii', suffix='.c') as handle:
        handle.file.write('\n'.join(contents))
        handle.file.write('\n')
        handle.file.flush()
        return compile_file(handle.name)
