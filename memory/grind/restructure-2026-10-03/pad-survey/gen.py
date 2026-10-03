"""Generate pad-free variants of display.c / text1b_tu1b.c / text1b_b.c in tmp/pad-survey/var/
with each pad moved to the end of its module's asm (tmp/pad-survey/asm/*.s copies)."""
import re, os, shutil
R = '.'
OUT = 'tmp/pad-survey/var'; ASM = 'tmp/pad-survey/asm'
os.makedirs(OUT, exist_ok=True)
# PAD site -> (predecessor, kind)  kind: inc (INCLUDE_ASM .s), c (C body -> whole asm), blk (file-scope glabel block)
PRED = {
 'display': None, 'text1b_tu1b': None, 'text1b_b': None}
C_TO_ASM = {'ReadGeomScreen': r's32 ReadGeomScreen\(void\) \{[^\n]*\}\n',
            'SetGeomOffset': r'void SetGeomOffset\(s32 a0, s32 a1\) \{.*?\n\}\n',
            'SetGeomScreen': r'void SetGeomScreen\(s32 a0\) \{.*?\n\}\n'}
def pad_s(name, n):
    src = open(f'asm/funcs/{name}.s', newline='').read()
    have = 1 if name == 'SetGeomScreen' else 0      # splat already kept REG13's pad
    add = ''.join('    nop\n' for _ in range(n - have))
    if f'endlabel {name}' in src:
        out = src.replace(f'endlabel {name}', add + f'endlabel {name}', 1)
    else:   # hand-written .s without endlabel (gte_ReadIR1IR2Sra2)
        out = src.rstrip('\n') + '\n' + add
    open(f'{ASM}/{name}.s', 'w', newline='\n').write(out)
report = []
for stem in PRED:
    text = open(f'src/{stem}.c', encoding='utf-8', newline='').read()
    lines = text.split('\n')
    sites = []
    for i, l in enumerate(lines):
        m = re.match(r'PAD_NOPS_(\d);', l)
        if not m: continue
        n = int(m.group(1))
        # predecessor: nearest previous non-comment code line
        j = i - 1
        while j >= 0 and lines[j] is not None and (lines[j].strip() == '' or lines[j].lstrip().startswith(('/*', '*'))): j -= 1
        prev = lines[j]
        if prev is None:   # stacked pads (func_800790A4)
            sites[-1][1] += n; lines[i] = None; continue
        mm = re.match(r'INCLUDE_ASM\("asm/funcs", (\w+)\);', prev)
        if mm: kind, name = 'inc', mm.group(1)
        elif prev.strip() == ');': kind, name = 'blk', '_patch_gte'
        else:
            # C function: find its name
            k = j
            while not re.match(r'^\w[\w\s\*]*\b(\w+)\(', lines[k]): k -= 1
            name = re.match(r'^\w[\w\s\*]*\b(\w+)\(', lines[k]).group(1); kind = 'c'
        sites.append([name, n, kind, i + 1]); lines[i] = None
    text = '\n'.join(l for l in lines if l is not None)
    text = re.sub(r'#define PAD_NOPS_\d __asm__\([^\n]*\)\n', '', text)
    for name, n, kind, ln in sites:
        report.append((stem, ln, name, n, kind))
        if kind == 'inc':
            pad_s(name, n)
            text = text.replace(f'INCLUDE_ASM("asm/funcs", {name});', f'INCLUDE_ASM("{ASM}", {name});', 1)
        elif kind == 'c':
            pad_s(name, n)
            text, k = re.subn(C_TO_ASM[name], f'INCLUDE_ASM("{ASM}", {name});\n', text, count=1, flags=re.S)
            assert k == 1, name
        else:
            old = r'    "    .word 0x40026800\n"' + '\n'
            assert old in text
            text = text.replace(old, old + r'    "    nop\n"' + '\n', 1)
    open(f'{OUT}/{stem}.c', 'w', encoding='utf-8', newline='\n').write(text)
for r in report: print(*r)
print(len(report), 'sites,', sum(r[3] for r in report), 'nops')
