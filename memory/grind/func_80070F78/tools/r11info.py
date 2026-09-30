"""r11info.py name... : for each dump, report the pseudos holding the Ruling 11 values
(sheets 0x74/0x60 loads, vram 0x7C loads, cells header+0xC/+0x24) with their lreg life line,
their global/local disposition and hard register, and ALLOCDBG priority."""
import re, sys
for name in sys.argv[1:]:
    lreg = open(f'dumps/{name}.lreg').read()
    greg = open(f'dumps/{name}.greg').read()
    instr = ''.join(l for l in open(f'dumps/{name}.instr') if 'func=func_80070F78 ' in l)
    disp = dict(re.findall(r'(\d+) in (\d+)', greg[greg.find(';; Register dispositions'):greg.find(';; Hard regs used')]))
    parts = re.split(r'\n(?=\((?:insn|call_insn|jump_insn|code_label|note|barrier) )', lreg)
    found = {}
    for p in parts:
        s = re.sub(r'\s+', ' ', p)
        m = re.match(r'\(insn (\d+) .*?\(set \(reg(?:/v)?:SI (\d+)\) \(mem:SI \(plus:SI \(reg:SI \d+\) \(const_int (116|96|124)\)\)\)\)', s)
        if m:
            found.setdefault({'116': 'sheets(0x74)', '96': 'sheets(0x60)', '124': 'vram(0x7C)'}[m.group(3)], []).append((m.group(1), m.group(2)))
        m = re.match(r'\(insn (\d+) .*?\(set \(reg/v:SI (\d+)\) \(plus:SI \(reg:SI \d+\) \(const_int (12|36)\)\)\)', s)
        if m:
            found.setdefault('cells(+%s)' % m.group(3), []).append((m.group(1), m.group(2)))
    print('==', name)
    for k, v in found.items():
        for uid, reg in v:
            life = re.search(r'\nRegister %s used [^\n]*' % reg, lreg)
            pri = re.search(r'pseudo=%s hardreg=(-?\d+) nrefs=(\d+) livelen=(\d+) pri=(\d+)' % reg, instr)
            print(f'  {k:14s} insn {uid:>5s} pseudo {reg:>4s} -> hard {disp.get(reg, "local/none")}  ' + (life.group(0).strip() if life else '') + (f'  [global pri={pri.group(4)} nrefs={pri.group(2)} livelen={pri.group(3)}]' if pri else '  [local-alloc]'))
