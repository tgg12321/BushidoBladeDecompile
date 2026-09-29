"""Collect Ruling 11 (D)(1) dump excerpts for func_8002A458 into dumps.txt.
Reads tmp/rtl/a458_<variant>_{stock,dbg,fr89}/ produced by dumps.sh / fr.sh."""
import re
from pathlib import Path

R = Path(__file__).resolve().parents[1] / 'rtl'
out = []


def w(s=''):
    out.append(s)


def reglines(v, regs):
    t = (R / ('a458_%s_stock' % v) / 'f.lreg').read_text()
    for r in regs:
        for line in t.splitlines():
            if line.startswith('Register %d ' % r):
                w('  ' + line)
        m = re.search(r'^;; Register %d in (\d+)\.$' % r, t, re.M)
        if m:
            w('  ;; Register %d in %s.   (local-alloc seat)' % (r, m.group(1)))


def alloc(v, regs):
    t = (R / ('a458_%s_dbg' % v) / 'stderr.txt').read_text()
    for line in t.splitlines():
        if 'func=func_8002A458 ' in line and any(' pseudo=%d ' % r in line for r in regs):
            w('  ' + line)


def ltu(v, dump):
    t = (R / ('a458_%s_stock' % v) / dump).read_text()
    m = re.search(r'\(ltu:SI \(reg/v:SI (\d+)\)', t)
    w('  %-7s first (ltu (reg %s) 1024) [the site-1 < 0x400 test]' % (dump, m.group(1)))


def findreg(v):
    t = (R / ('a458_%s_fr89' % v) / 'stderr.txt').read_text()
    i = t.index('FINDREGDBG func=func_8002A458')
    for line in t[i:].splitlines()[:9]:
        w('  ' + line)


w('# func_8002A458 -- Ruling 11 (D)(1) dump excerpts (manual session 2, 2026-09-28)')
w('# Produced by r11/dumps.sh and r11/fr.sh (copies of tmp/func_8002A458/*.sh), mk.py compiles the')
w('# variant spliced into src/code6cac_b.c with the build flags (engine.buildconfig CC_FLAGS) and -da.')
w('# stock = tools/gcc-2.7.2/build/cc1 (the build compiler); dbg = tools/gcc-2.7.2/cc1 (instrumented,')
w('# BB2_ALLOC_DEBUG / BB2_FINDREG_DEBUG=89). dumps.sh checks each dbg .s is identical to the stock .s:')
w('#   final / onevar_full / dxyz_all_own / temp_all_own / temp2_split: stock==dbg asm (all five).')
w('# Variants: r11v/<name>.c (final = the submitted body; the others differ only in declarations/identifiers).')
w()
w('## dx / dy / dz')
w('final (reuse): pseudos 83 = dx, 84 = dy, 85 = dz')
reglines('final', [83, 84, 85])
alloc('final', [83, 84, 85])
w('dxyz_all_own (hit/obj values in their own locals hx..oz): the six end-block pseudos')
reglines('dxyz_all_own', [272, 273, 274, 275, 276, 277])
w('  (hard reg 2 = $v0; the target seats the end-block deltas in $s2/$s3/$s1 = 18/19/17)')
w()
w('## temp (squared horizontal length, then the segment length)')
w('final (reuse): temp = 87, temp2 = 89 (island input)')
ltu('final', 'f.rtl')
ltu('final', 'f.cse')
reglines('final', [87, 89])
w('temp_all_own (squared length in its own h_sq = 90; temp2 = 89):')
ltu('temp_all_own', 'f.rtl')
ltu('temp_all_own', 'f.cse')
reglines('temp_all_own', [89, 90])
w('  -> cse1 replaced the test operand h_sq (90) by the copy temp2 (89): the copy became the')
w('     canonical register of the quantity (cse.c make_regs_eqv :844-857).')
w()
w('## temp2 (the gte_ldlzc input copy, then the table byte)')
w('final (reuse): find_reg for pseudo 89')
findreg('final')
alloc('final', [89])
w('temp2_split (copy in its own n = 89, table byte in a block-scope tbl):')
findreg('temp2_split')
alloc('temp2_split', [89])
t = (R / 'a458_final_stock' / 'f.lreg').read_text()
m = re.search(r'\(insn \d+ [^\n]*\(set \(reg:SI 139\)\n[^\n]*', t)
w('final: the reader that gives the full preference (f.lreg):')
w('  ' + m.group(0).replace('\n', '\n  '))
reglines('final', [133, 134, 135, 139])
w('  (139 local-allocated to hard reg 4 = $a0; 133-135, the lz/shift code inside temp2\'s range, to 3 = $v1)')
(Path(__file__).resolve().parent / 'dumps.txt').write_text('\n'.join(out) + '\n')
print('\n'.join(out))
