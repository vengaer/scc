import cc

def test_unary_static_assert_not_triggered(root):
    ec, stdout, stderr = cc.compile_supplied([
       f'#include <{root}/scc/bug.h>',
        'int main(void) {',
        '    scc_static_assert(1 == 1);',
        '}',
    ])

    assert not stderr
    assert ec == 0

def test_unary_static_assert_triggered(root):
    ec, stdout, stderr = cc.compile_supplied([
       f'#include <{root}/scc/bug.h>',
        'int main(void) {',
        '   scc_static_assert(1 == 0);',
        '}',
    ])
    assert ec != 0

def test_binary_static_assert_not_triggered(root):
    ec, stdout, stderr = cc.compile_supplied([
       f'#include <{root}/scc/bug.h>',
        'int main(void) {',
        '   scc_static_assert(1, "This should not trigger");',
        '}'
    ])
    assert not stderr
    assert ec == 0

def test_binary_static_assert_triggered(root):
    ec, stdout, stderr = cc.compile_supplied([
       f'#include <{root}/scc/bug.h>',
        'int main(void) {',
        '   scc_static_assert(0, "This should trigger");',
        '}',
    ])
    assert ec != 0
