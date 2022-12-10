import cc

def test_unary_static_assert_not_triggered(root):
    ec, stdout, stderr = cc.compile_supplied([
       f'#include <{root}/scc/scc_dbg.h>',
        'scc_static_assert(1 == 1);',
        'int main(void) { }'
    ])

    assert not stderr
    assert ec == 0

def test_unary_static_assert_triggered(root):
    ec, stdout, stderr = cc.compile_supplied([
       f'#include <{root}/scc/scc_dbg.h>',
        'scc_static_assert(1 == 0);',
        'int main(void) { }'
    ])
    assert ec != 0

def test_binary_static_assert_not_triggered(root):
    ec, stdout, stderr = cc.compile_supplied([
       f'#include <{root}/scc/scc_dbg.h>',
        'scc_static_assert(1, "This should not trigger");',
        'int main(void) { }'
    ])
    assert not stderr
    assert ec == 0

def test_binary_static_assert_triggered(root):
    ec, stdout, stderr = cc.compile_supplied([
       f'#include <{root}/scc/scc_dbg.h>',
        'scc_static_assert(0, "This should trigger");',
        'int main(void) { }'
    ])
    assert ec != 0
