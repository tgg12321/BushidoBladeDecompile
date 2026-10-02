"""gen.py <in.t> [<out.c>] : expand GTE_SETROT(x); GTE_LDV0(x); GTE_RTV0(); GTE_STLVNL(x); lines
into the verbatim inline_o.h statement runs (same text as func_800204C0 on main)."""
import re
import sys

CL = ':"$12","$13","$14","$15","memory");'


def st(ind, text, operand=None):
    if operand is None:
        return f'{ind}__asm__ volatile ("{text}": : {CL}'
    return f'{ind}__asm__ volatile ("{text}": :"r"({operand}){CL}'


def expand(line):
    m = re.match(r'^(\s*)GTE_(SETROT|LDV0|RTV0|STLVNL)\((.*)\);\s*$', line)
    if not m:
        return [line]
    ind, kind, arg = m.groups()
    if kind == 'SETROT':
        out = [f'{ind}/* inline_o.h: gte_SetRotMatrix :272-284 */',
               st(ind, 'move  $12,%0', arg)]
        for t in ['lw    $13,($12)', 'lw    $14,4($12)', 'ctc2  $13,$0', 'ctc2  $14,$1',
                  'lw    $13,8($12)', 'lw    $14,12($12)', 'lw    $15,16($12)',
                  'ctc2  $13,$2', 'ctc2  $14,$3', 'ctc2  $15,$4']:
            out.append(st(ind, t))
        return out
    if kind == 'LDV0':
        return [f'{ind}/* inline_o.h: gte_ldv0 :16-20 */', st(ind, 'move  $12,%0', arg),
                st(ind, 'lwc2  $0,($12)'), st(ind, 'lwc2  $1,4($12)')]
    if kind == 'RTV0':
        return [f'{ind}/* inline_o.h: gte_rtv0 :426-430, post-DMPSX command word (see above) */', st(ind, 'nop   '),
                st(ind, 'nop   '), st(ind, '.word 0x4A486012')]
    return [f'{ind}/* inline_o.h: gte_stlvnl :904-909 */', st(ind, 'move  $12,%0', arg),
            st(ind, 'swc2  $25,($12)'), st(ind, 'swc2  $26,4($12)'), st(ind, 'swc2  $27,8($12)')]


src = open(sys.argv[1]).read().split('\n')
out = []
for ln in src:
    out.extend(expand(ln))
dst = sys.argv[2] if len(sys.argv) > 2 else sys.argv[1][:-2] + '.c'
open(dst, 'w', newline='\n').write('\n'.join(out))
