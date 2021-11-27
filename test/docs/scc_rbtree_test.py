import re

from fixtures import *

def test_scc_rbtree_example(uniproc):
    assert uniproc[0] == 0

    s = ''.join(filter(lambda l: re.search('^[0-9 ]+$', l), uniproc[1]))
    nums = list(map(int, s.strip().split(' ')))
    assert sorted(nums) == nums

def test_scc_rbtree_foreach(uniproc):
    assert uniproc[0] == 0
    n = list(range(0, 39, 2))
    assert n == list(map(int, uniproc[1].split(' ')))

def test_scc_rbtree_foreach_reversed(uniproc):
    assert uniproc[0] == 0
    n = list(map(int, uniproc[1].split(' ')))
    assert n == list(reversed(sorted(n)))
