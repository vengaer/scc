''' Tests related to directory tree traversal '''

import os
import pathlib
import shutil
import subprocess
import tempfile
import pytest

import make
from fixtures import project_root, script_dir

def _group_debug_output(stdout, maxdepth, root):
    ''' Group debug output after directory '''
    dbg = list(filter(lambda s: s.startswith('DEBUG:'), stdout))
    stack = [root]
    grouped = {}
    grouped[stack[-1]] = []
    for line in dbg:
        line = line.replace('DEBUG: ', '')
        if 'enter ' in line:
            stack.append(line.replace('enter ', '').strip())
            grouped['/'.join(stack)] = []
        elif 'exit ' in line:
            node = line.replace('exit ', '').strip()
            assert stack[-1] == node
            stack.pop()

        grouped['/'.join(stack)].append(line)

        # Depth should be at most maxdepth
        assert len(stack) <= maxdepth
    return grouped

def _write_makefile(path, contents):
    ''' Write but don't run makefile '''
    with open(str(path / 'Makefile'), 'w', encoding='ascii') as handle:
        handle.write('\n'.join(contents))
        handle.write('\n')

def _supports_mkdir():
    ''' Check whether the OS supports mkdir, the brutish way '''
    exists = False
    with tempfile.TemporaryDirectory() as tmpdir:
        testdir = f'{tmpdir}/test'
        try:
            exists = subprocess.run(['mkdir', '-p', testdir], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL).returncode == 0
        finally:
            if os.path.exists(testdir):
                shutil.rmtree(testdir)
    return exists

@pytest.mark.parametrize('explicit_builddir', [False, True])
def test_flat_tree_traversal(script_dir, explicit_builddir):
    ''' Create a directory tree with Makefiles and verify that its traversed correctly '''
    subdirs = ['a', 'b', 'c', 'd']

    with tempfile.TemporaryDirectory() as tmpdir:
        for subdir in subdirs:
            path = pathlib.Path(tmpdir) / subdir
            path.mkdir()
            makefile = path / 'Makefile'
            makefile.touch()

        ec, stdout, stderr = make.make_supplied([
           f'root      := {tmpdir}',
            'builddir  := ${root}/build',
           f'mkscripts := {script_dir}',
            'include $(mkscripts)/node.mk',
           f'$(foreach __s,{" ".join(subdirs)},$(call include-node,$(__s){", build_$(__s)" if explicit_builddir else ""}))',
            '.PHONY: all',
            'all:'
        ], args='DEBUG=1', directory=tmpdir)

        assert ec == 0

        grouped = _group_debug_output(stdout, 2, tmpdir)

        # Verify automatic variables
        for subdir in subdirs:
            build_basename = subdir if not explicit_builddir else f'build_{subdir}'
            assert f'exit {subdir}' in grouped[tmpdir]
            assert f'enter {subdir}' in grouped[f'{tmpdir}/{subdir}']
            assert f'__node_builddir: {tmpdir}/build/{build_basename}' in grouped[f'{tmpdir}/{subdir}']
            assert f'__node_path: {tmpdir}/{subdir}' in grouped[f'{tmpdir}/{subdir}']

        # Should occur once for each time the root is returned to
        for info in ['__node_path', '__node_builddir']:
            assert len(list(filter(lambda s: f'{info}:' in s, grouped[tmpdir]))) == len(subdirs)

        # Automatic variables should be the same for the root each time
        assert len(set(grouped[tmpdir])) == len(grouped[tmpdir]) / len(subdirs) + len(subdirs) - 1

def test_nested_tree_traversal(script_dir):
    ''' Generate a nested directory tree and verify the correctness '''

    lvl1_dirs = ['a', 'b', 'c', 'd']
    lvl2_dirs = ['e', 'f', 'g', 'h']
    lvl3_dirs = ['i', 'j', 'k', 'l']

    with tempfile.TemporaryDirectory() as tmpdir:
        root = pathlib.Path(tmpdir)
        for l1d in lvl1_dirs:
            l1d = root / l1d
            l1d.mkdir()
            for l2d in lvl2_dirs:
                l2d = l1d / l2d
                l2d.mkdir()
                for l3d in lvl3_dirs:
                    l3d = l2d / l3d
                    l3d.mkdir()
                    (l3d / 'Makefile').touch()
                _write_makefile(l2d, [ f'$(foreach __s,{" ".join(lvl3_dirs)},$(call include-node, $(__s)))' ])
            _write_makefile(l1d, [ f'$(foreach __s,{" ".join(lvl2_dirs)},$(call include-node, $(__s)))' ])

        ec, stdout, stderr = make.make_supplied([
           f'root      := {tmpdir}',
            'builddir  := ${root}/build',
           f'mkscripts := {script_dir}',
            'include $(mkscripts)/node.mk',
           f'$(foreach __s,{" ".join(lvl1_dirs)},$(call include-node,$(__s)))',
            '.PHONY: all',
            'all:'
        ], args='DEBUG=1', directory=tmpdir)

        assert ec == 0
        grouped = _group_debug_output(stdout, 4, tmpdir)

        path = pathlib.Path(tmpdir)
        # Verify automatic variables
        for l1d in lvl1_dirs:
            l1d = path / l1d
            assert f'enter {l1d.name}' in grouped[str(l1d)]
            assert f'__node_builddir: {tmpdir}/build/{l1d.name}' in grouped[str(l1d)]
            assert f'__node_path: {tmpdir}/{l1d.name}' in grouped[str(l1d)]

            for l2d in lvl2_dirs:
                l2d = l1d / l2d
                assert f'enter {l2d.name}' in grouped[str(l2d)]
                assert f'__node_builddir: {tmpdir}/build/{l1d.name}/{l2d.name}' in grouped[str(l2d)]
                assert f'__node_path: {tmpdir}/{l1d.name}/{l2d.name}' in grouped[str(l2d)]

                for l3d in lvl3_dirs:
                    l3d = l2d / l3d
                    assert f'enter {l3d.name}' in grouped[str(l3d)]
                    assert f'__node_builddir: {tmpdir}/build/{l1d.name}/{l2d.name}/{l3d.name}' in grouped[str(l3d)]
                    assert f'__node_path: {tmpdir}/{l1d.name}/{l2d.name}/{l3d.name}' in grouped[str(l3d)]

                for info in ['__node_path', '__node_builddir']:
                    # Should occur once for __exit_node and once for each __enter_node
                    assert len(list(filter(lambda s: f'{info}:' in s, grouped[str(l2d)]))) == len(lvl3_dirs) + 1

                # Automatic variables should be the same each time
                assert len(set(grouped[str(l2d)])) == len(grouped[str(l2d)]) / len(lvl3_dirs) + len(lvl3_dirs)

            for info in ['__node_path', '__node_builddir']:
                # Should occur once for __exit_node and once for each __enter_node
                assert len(list(filter(lambda s: f'{info}:' in s, grouped[str(l1d)]))) == len(lvl2_dirs) + 1
            # Automatic variables should be the same each time
            assert len(set(grouped[str(l1d)])) == len(grouped[str(l1d)]) / len(lvl2_dirs) + len(lvl2_dirs)

            for info in ['__node_path', '__node_builddir']:
                # No __enter_node for root
                assert len(list(filter(lambda s: f'{info}:' in s, grouped[tmpdir]))) == len(lvl1_dirs)
            # No __enter_node for root
            assert len(set(grouped[str(l1d)])) == len(grouped[str(l1d)]) / len(lvl2_dirs) + len(lvl2_dirs)

@pytest.mark.skipif(not _supports_mkdir(), reason='Relies on mkdir command')
def test_builddirs_generated(script_dir):
    ''' Make sure that build directories are generated as required '''
    dirchain = ['a', 'b', 'c']
    subdirs = ['d', 'e']

    with tempfile.TemporaryDirectory() as tmpdir:
        top = pathlib.Path(tmpdir)
        for subdir in subdirs:
            subdir = top / subdir
            subdir.mkdir()
            _write_makefile(subdir, [
                'all: | $(__node_builddir)',
               f'$(call include-node,{dirchain[0]})'
            ])
            chain = subdir
            for i, chaindir in enumerate(dirchain):
                chain = chain / chaindir
                chain.mkdir()
                _write_makefile(chain, [
                    f'$(call include-node,{dirchain[i + 1]})' if i < len(dirchain) - 1 else '',
                    'all: | $(__node_builddir)'
                ])

        ec, stdout, stderr = make.make_supplied([
           f'root      := {tmpdir}',
            'builddir  := ${root}/build',
           f'mkscripts := {script_dir}',
            'include $(mkscripts)/node.mk',
            'dirs += $(root)',
            '.PHONY: all',
            'all:',
            '',
           f'$(foreach __s,{" ".join(subdirs)},$(call include-node,$(__s)))',
            '$(dirs):',
            '\tmkdir -p $@'
        ], args='DEBUG=1', directory=tmpdir)

        assert ec == 0
        grouped = _group_debug_output(stdout, 5, tmpdir)
        builddirs = []
        for dbg in grouped.values():
            for ent in set(dbg):
                if ent.startswith('__node_builddir'):
                    builddirs.append(ent.replace('__node_builddir: ', '').strip())
        assert builddirs
        for builddir in builddirs:
            assert os.path.exists(builddir)

def test_node_objects_restored_on_exit(script_dir):
    ''' Verify that node-local objects are restored when exiting substate '''

    subdirs = ['a', 'b', 'c', 'd']
    exp_objs = 'root.o main.o abcd.o'

    with tempfile.TemporaryDirectory() as tmpdir:
        for subdir in subdirs:
            path = pathlib.Path(tmpdir) / subdir
            path.mkdir()
            _write_makefile(path, [
               f'__node_obj := {subdir}.o'
            ])

        ec, stdout, stderr = make.make_supplied([
           f'root       := {tmpdir}',
            'builddir   := ${root}/build',
           f'mkscripts  := {script_dir}',
           f'__node_obj := {exp_objs}',
            'include $(mkscripts)/node.mk',
           f'$(foreach __s,{" ".join(subdirs)},$(call include-node,$(__s)))',
            '$(info root objects: $(__node_obj))',
            '.PHONY: all',
            'all:'
        ], args='DEBUG=1', directory=tmpdir)

        assert ec == 0
        assert f'root objects: {exp_objs}' in stdout

        grouped = _group_debug_output(stdout, 2, tmpdir)
        assert f'restored obj: {exp_objs}' in grouped[tmpdir]
