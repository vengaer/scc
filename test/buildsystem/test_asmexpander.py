''' Tests for the asmexpander script '''

import sys
import tempfile

from fixtures import *
@pytest.mark.parametrize('asm, exp', [
    (
        [
            '.macro do_andnl r0, r1',
           r'   andnl \r0, \r1',
            '.endm',
            '',
            'foo:',
            '   do_andnl %ecx, %eax'
        ],
        [
            'foo:',
            'andnl %ecx, %eax'
        ],
    ),
    (
        [
            '.macro do_subq r0, r1',
           r'   subq \r0, \r1',
            '.endm',
            '.macro do_addq r0, r1',
           r'   addq \r0, \r1',
            '.endm',
            '.macro do_ident r0, r1',
           r'   do_addq \r0, \r1',
           r'   do_subq \r0, \r1',
            '.endm',
            'foo:',
            '   do_ident %eax, %ecx'
        ],
        [
            'foo:',
            'addq %eax, %ecx',
            'subq %eax, %ecx'
        ]
    ),
    (
        [
            '.macro do_if c',
           r'.if \c',
            '   orl $0xff, %eax',
            '.endif',
            '.endm',
            'foo:',
            '   do_if 1'
        ],
        [
            'foo:',
            'orl $0xff, %eax'
        ]
    ),
    (
        [
            '.macro do_if c',
           r'.if \c',
            '   orl $0xff, %eax',
            '.endif',
            '.endm',
            'foo:',
            '   do_if 0'
        ],
        [
            'foo:'
        ]
    ),
    (
        [
            '.macro do_else c',
           r'.if \c',
            '   andl $0xff, %eax',
            '.else',
            '   orl $0xff, %eax',
            '.endif',
            '.endm',
            'foo:',
            '   do_else 0'
        ],
        [
            'foo:',
            'orl $0xff, %eax'
        ]
    ),
    (
        [
            '.macro do_else c',
           r'.if \c',
            '   andl $0xff, %eax',
            '.else',
            '   orl $0xff, %eax',
            '.endif',
            '.endm',
            'foo:',
            '   do_else 1'
        ],
        [
            'foo:',
            'andl $0xff, %eax',
        ]
    )
])
def test_asmexpand(asm, exp, build_dir, pyscripts_in_path):
    import asmexpander

    outfile = f'{build_dir}/.asmexpand'
    with tempfile.NamedTemporaryFile(mode='w', encoding='ascii') as handle:
        handle.write('\n'.join(asm + ['']))
        handle.flush()
        asmexpander.main(handle.name, outfile, False)

    try:
        with open(outfile, 'r', encoding='ascii') as handle:
            expanded = list(map(lambda s: s.strip(), handle.readlines()))
        expanded = list(filter(lambda s: s, expanded))

        assert expanded == exp

    finally:
        os.unlink(outfile)
