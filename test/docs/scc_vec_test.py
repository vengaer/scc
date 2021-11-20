import pytest

from fixtures import *

@pytest.mark.parametrize('biproc', ['1\n2\n3\n'], indirect=True)
def test_scc_vec_example(biproc):
    assert biproc[0] == 0
    assert 'You entered: 1 2 3' in biproc[1]

def test_scc_vec_erase(uniproc):
    assert uniproc[0] == 0
    assert list(filter(lambda v: v != 9, range(0, 11))) == [int(v) for v in uniproc[1].split(' ')]

def test_scc_vec_erase_range(uniproc):
    assert uniproc[0] == 0
    assert list(range(0,3)) + list(range(8,10)) == [int(v) for v in uniproc[1].split(' ')]

def test_scc_vec_foreach(uniproc):
    assert uniproc[0] == 0
    assert [1, 2, 3] == [int(v) for v in uniproc[1].split(' ')]

def test_scc_vec_foreach_by(uniproc):
    assert uniproc[0] == 0
    assert [8, 7] == [int(v) for v in uniproc[1].split(' ')]

def test_scc_vec_foreach_reversed(uniproc):
    assert uniproc[0] == 0
    assert [0, 12, 2] == [int(v) for v in uniproc[1].split(' ')]

def test_scc_vec_foreach_reversed_by(uniproc):
    assert uniproc[0] == 0
    assert [11, 5] == [int(v) for v in uniproc[1].split(' ')]

def test_scc_vec_from(nullproc):
    assert nullproc == 0

def test_scc_vec_from_emulation(nullproc):
    assert nullproc == 0

def test_scc_vec_init_int(nullproc):
    assert nullproc == 0

def test_scc_vec_init_point(nullproc):
    assert nullproc == 0

def test_scc_vec_reserve(nullproc):
    assert nullproc == 0
