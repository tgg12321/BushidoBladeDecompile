"""Assemble hand-written probes with the real ASPSX 2.34 and decode .text.
usage (WSL, repo root): python3 tmp/research36140/run_asm_tests.py"""
import subprocess, sys
from pathlib import Path
sys.path.insert(0, 'tools/maspsx/aspsx')
import util

HERE = Path('tmp/research36140')
PROBES = {
    'comm_after':  ['	.text', '	lb	$4,ca', '	lb	$5,ca+1', '	.comm	ca,4'],
    'lcomm_after': ['	.text', '	lb	$4,la_', '	lb	$5,la_+1', '	.lcomm	la_,4'],

    'comm4':      ['\t.comm\tcv,4', '\t.text', '\tlb\t$4,cv', '\tlb\t$5,cv+1', '\tsb\t$6,cv+2'],
    'lcomm4':     ['\t.lcomm\tlv,4', '\t.text', '\tlb\t$4,lv', '\tlb\t$5,lv+1', '\tsb\t$6,lv+2'],
    'extern4':    ['\t.extern\tev,4', '\t.text', '\tlb\t$4,ev', '\tlb\t$5,ev+1', '\tsb\t$6,ev+2'],
    'extern4_end': ['\t.text', '\tlb\t$4,ev', '\tlb\t$5,ev+1', '\tsb\t$6,ev+2', '\t.extern\tev,4'],
    'undeclared': ['\t.text', '\tlb\t$4,uv', '\tlb\t$5,uv+1'],
    'sdata4':     ['\t.sdata', 'sv:', '\t.word\t0', '\t.text', '\tlb\t$4,sv', '\tlb\t$5,sv+1'],
    'globl_comm': ['\t.globl\tgc', '\t.comm\tgc,4', '\t.text', '\tlb\t$4,gc', '\tlb\t$5,gc+1'],
    'comm8_off4': ['\t.comm\tc8,8', '\t.text', '\tlw\t$4,c8', '\tlw\t$5,c8+4'],
    'extern8_off4': ['\t.extern\te8,8', '\t.text', '\tlw\t$4,e8', '\tlw\t$5,e8+4'],
}

def decode(w):
    op = w >> 26; rs = (w >> 21) & 31
    if op == 0x0F:
        return f'lui  r{(w >> 16) & 31}'
    names = {0x20: 'lb', 0x24: 'lbu', 0x23: 'lw', 0x28: 'sb', 0x2B: 'sw', 0x21: 'lh', 0x25: 'lhu', 0x29: 'sh'}
    n = names.get(op, f'op{op:x}')
    return f'{n:4} base=r{rs}' + (' (GP)' if rs == 28 else '')

for name, lines in (PROBES.items() if __name__ == '__main__' else []):
    s = HERE / f'p_{name}.s'
    s.write_text('\n'.join(lines) + '\n')
    o = HERE / f'p_{name}.obj'
    r = subprocess.run(['bash', str(HERE / 'aspsx.sh'), str(s), str(o), '-G8'], capture_output=True, text=True)
    if r.returncode:
        print(f'{name}: FAIL {r.stderr.strip()[:300]}'); continue
    try:
        t = util.read_text_section(o.read_bytes())
    except Exception as e:
        print(f'{name}: parse error {e}'); continue
    ws = [int.from_bytes(t[i:i + 4], 'little') for i in range(0, len(t), 4)]
    print(f'{name}: ' + ' | '.join(decode(w) for w in ws if w))
