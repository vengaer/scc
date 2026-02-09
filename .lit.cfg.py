import inspect
import os
import pathlib

import lit.formats

config.name = 'scc'
config.test_format = lit.formats.ShTest('0')

config.suffixes = {'.c'}

config.excludes = ['arch', 'benchmark', 'build', 'lib', 'scc', 'scripts', 'submodules', 'test']

_config = inspect.getfile(inspect.currentframe())

_root = pathlib.Path(_config).resolve().parent
_builddir = _root / 'build'
_litbuild = _builddir / 'lit'
_filecheck = _root / 'submodules' / 'filecheck' / 'filecheck' / 'filecheck.py'
_cflags = '-g'
_cppflags = f'-I{_root}'
_ldlibs = '-lscc -lm'
_ldflags = f'-L{_builddir}'
_libscc_a = _builddir / 'libscc.a'
_ldlib_path = _builddir

_litbuild.mkdir(parents=True, exist_ok=True)

config.substitutions.append(('%cc', 'clang'))
config.substitutions.append(('%filecheck', str(_filecheck)))
config.substitutions.append(('%dynamic', f'{_cflags} {_cppflags} {_ldflags} {_ldlibs}'))
config.substitutions.append(('%static', f'{_cflags} {_cppflags} {_libscc_a} {_ldlibs.replace("-lscc", "")}'))

config.environment['LD_LIBRARY_PATH'] = _ldlib_path

# Execute tests in build dir
config.test_exec_root = _litbuild
