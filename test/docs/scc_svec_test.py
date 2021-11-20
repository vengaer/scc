import pytest

from fixtures import *

@pytest.mark.parametrize('biproc', ['this is a test string'], indirect=True)
def test_scc_svec_example(biproc):
    assert biproc[0] == 0
    assert 'thisisateststring' in biproc[1]

def test_scc_svec_erase(uniproc):
    assert uniproc[0] == 0
    assert list(range(1, 11)) == [int(v) for v in uniproc[1].split(' ')]

def test_scc_svec_erase_range(uniproc):
    assert uniproc[0] == 0
    assert list(range(0, 8)) == [int(v) for v in uniproc[1].split(' ')]

def test_scc_svec_foreach(uniproc):
    assert uniproc[0] == 0
    assert list(range(1,5)) == [int(v) for v in uniproc[1].split(' ')]

def test_scc_svec_foreach_by(uniproc):
    assert uniproc[0] == 0
    assert [8, 62] == [int(v) for v in uniproc[1].split(' ')]

def test_scc_svec_foreach_reversed(uniproc):
    assert uniproc[0] == 0
    assert [0, 1, 0, 12, 2] == [int(v) for v in uniproc[1].split(' ')]

def test_scc_svec_foreach_reversed_by(uniproc):
    assert uniproc[0] == 0
    assert [11, 5] == [int(v) for v in uniproc[1].split(' ')]
