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
_filecheck = _root / 'submodules' / 'filecheck' / 'filecheck' / 'filecheck.py'
_libscc_a = _builddir / 'libscc.a'
_litbuild = _builddir / 'lit'

_litbuild.mkdir(parents=True, exist_ok=True)

config.substitutions.append(('%cc', 'clang'))
config.substitutions.append(('%filecheck', _filecheck))
config.substitutions.append(('%root', _root))
config.substitutions.append(('%libscc_a', _libscc_a))
config.substitutions.append(('%litbuild', _litbuild))

# Execute tests in build dir
config.test_exec_root = _litbuild
