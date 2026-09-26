"""Aggregate-merge prong (c) for the func_80036140 landing: retire the per-word symbol rows the merges
replace (no C code and no still-INCLUDE_ASM function names them afterwards; the full build + link proves
it) and correct the named_syms.txt span comment.
usage: python3 tmp/func_80036140/symfiles.py <tree>"""
import re, sys, glob
from pathlib import Path

R = Path(sys.argv[1])
RETIRE = {
    # CdlATV merges (D_800A36B8, g_cd_atv)
    'D_800A36B9', 'D_800A36BA', 'D_800A36BB', 'g_cd_atv_plus_0x1', 'g_cd_atv_plus_0x2', 'g_cd_atv_plus_0x3',
    'g_special_cam_win_byte_a_plus_1', 'g_special_cam_win_byte_a_plus_2', 'g_special_cam_win_byte_a_plus_3',
    # CdState rows kept only for asm/funcs/func_80036140.s ("retire with func_80036140")
    'D_80101E62', 'D_80101E64', 'D_80101E68', 'D_80101E6A', 'g_cd_loc', 'D_80101E74', 'D_80101E88',
    'D_80101E8C', 'D_80101E90', 'D_80101E94', 'D_80101E9A',
    # the CdState extension 0x80101E9C..0x80101EA7
    'D_80101E9C', 'D_80101E9E', 'g_cdread_expected_pos', 'D_80101EA4',
    'g_cdread_state_9C_80101E9C', 'g_c6cb2_state_E9E',
}
row = re.compile(r'^\s*([A-Za-z_]\w*)\s*=\s*0x[0-9A-Fa-f]+\s*;')
removed = {}
for rel in ('undefined_syms_auto.txt', 'named_syms.txt', 'symbol_addrs.txt', 'sdata_syms.txt'):
    p = R / rel
    out, n = [], []
    for line in p.read_text(encoding='utf-8').split('\n'):
        m = row.match(line)
        name = m.group(1) if m else line.strip()
        if name in RETIRE:
            n.append(name)
            continue
        out.append(line)
    s = '\n'.join(out)
    if rel == 'named_syms.txt':
        old = '/* 2026-09-26: 0x80101E58..0x80101E9B is ONE object, CdState D_80101E58 */'
        assert s.count(old) == 1
        s = s.replace(old, '/* 2026-09-26: 0x80101E58..0x80101EA7 is ONE object, CdState D_80101E58 */')
    p.write_bytes(s.encode('utf-8'))
    removed[rel] = n
# the splat data asm: one dlabel per CdlATV (its per-byte labels retire; the bytes are unchanged)
p = R / 'asm/data/91C98.data.s'
s = p.read_text(encoding='utf-8')
for base, subs in (('D_800A36B8', ('D_800A36B9', 'D_800A36BA', 'D_800A36BB')),
                   ('g_cd_atv', ('g_cd_atv_plus_0x1', 'g_cd_atv_plus_0x2', 'g_cd_atv_plus_0x3'))):
    prev = base
    for sub in subs:
        old = f'enddlabel {prev}\n\nnonmatching {sub}\n\ndlabel {sub}\n'
        assert s.count(old) == 1, old
        s = s.replace(old, '')
        prev = sub
    old = f'enddlabel {subs[-1]}\n'
    assert s.count(old) == 1
    s = s.replace(old, f'enddlabel {base}\n')
p.write_bytes(s.encode('utf-8'))
removed['asm/data/91C98.data.s'] = ['dlabels D_800A36B9..BB -> D_800A36B8, g_cd_atv_plus_0x1..3 -> g_cd_atv']
for k, v in removed.items():
    print(f'{k}: removed {len(v)}: {sorted(v)}')
# nothing built may still name a retired symbol
srcs = [Path(f) for f in glob.glob(str(R / 'src/*.c')) + glob.glob(str(R / 'include/*.h'))]
live_asm = set()
for f in glob.glob(str(R / 'src/*.c')):
    live_asm |= set(re.findall(r'INCLUDE_ASM\("asm/funcs", (\w+)\)', Path(f).read_text()))
for fn in live_asm:
    srcs.append(R / f'asm/funcs/{fn}.s')
srcs += [Path(f) for f in glob.glob(str(R / 'asm/data/*.s'))]
bad = [(str(f), s) for f in srcs for s in RETIRE if re.search(r'\b' + s + r'\b', f.read_text(errors='replace'))]
print('still named by a built source:', bad or 'none')
assert not bad
