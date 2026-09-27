"""Q14 clause 2: per-accessor byte identity (expected_pos s32 vs u32) — function bytes + SHA1 from each
build's object, with the exact per-file recipes. Clause 1: the no-reveal listing of the original accesses.
usage (WSL, repo root): python3 tmp/func_80036140/q14_accessors.py > memory/grind/func_80036140/landing/q14_accessors.txt"""
import hashlib, re, subprocess

H = 'tmp/func_80036140'
ACC = [('cdrom_ReadyCallback', 'code6cac_b4_post'), ('func_80036940', 'code6cac_b5')]


def func_bytes(o, f):
    syms = subprocess.run(['mipsel-linux-gnu-objdump', '-t', o], capture_output=True, text=True).stdout
    tab = {}
    for l in syms.splitlines():
        m = re.match(r'^([0-9a-f]{8})\s+\w?\s*\w?\s+F?\s*\.text\s+([0-9a-f]{8})\s+(\S+)$', l)
        if m:
            tab[m.group(3)] = (int(m.group(1), 16), int(m.group(2), 16))
    subprocess.run(['mipsel-linux-gnu-objcopy', '-O', 'binary', '-j', '.text', o, '/tmp/q14.bin'], check=True)
    text = open('/tmp/q14.bin', 'rb').read()
    offs = sorted(v[0] for v in tab.values())
    start, size = tab[f]
    if not size:
        nxt = [x for x in offs if x > start]
        size = (nxt[0] if nxt else len(text)) - start
    return text[start:start + size]


print('# Q14 clause 2 — per-accessor byte identity, expected_pos declared s32 vs u32')
print('# trees: tmp/func_80036140/fin_s32, fin_u32 (identical except include/code6cac.h: `s32|u32 expected_pos;`)')
for f, stem in ACC:
    print(f'\n## {f} ({stem}.c)')
    for t in ('fin_s32', 'fin_u32'):
        rec = subprocess.run(['make', '-C', f'{H}/{t}', '-n', '-B', f'build/src/{stem}.o'], capture_output=True, text=True).stdout
        cmd = [l for l in rec.splitlines() if f'-o build/src/{stem}.o' in l][-1]
        b = func_bytes(f'{H}/{t}/build/src/{stem}.o', f)
        print(f'  {t}: {len(b)} bytes  sha1 {hashlib.sha1(b).hexdigest()}')
        print(f'    recipe (run in {H}/{t}): {cmd}')
    o1 = open(f'{H}/fin_s32/build/src/{stem}.o', 'rb').read()
    o2 = open(f'{H}/fin_u32/build/src/{stem}.o', 'rb').read()
    print(f'  whole object sha1: s32 {hashlib.sha1(o1).hexdigest()}  u32 {hashlib.sha1(o2).hexdigest()}')

print('\n# Q14 clause 1 — every original access to 0x80101EA0 and every instruction its value flows into (asm/funcs)')
print('''cdrom_ReadyCallback 800360A0 lui v1 / 800360A4 lw v1,%lo(g_cdread_expected_pos)(v1)
    -> 800360AC bne v0,v1  (equality test; its last use on both paths: fall-through overwrites v1 at 800360D0 lw, the taken path .L80036114 never reads v1 before jal CdReadyCallback)
cdrom_ReadyCallback 800360E0 lui v0 / 800360E4 lw v0,%lo(g_cdread_expected_pos)(v0)
    -> 800360F4 addiu v0,v0,1 -> 800360FC sw v0,%lo(g_cdread_expected_pos)(at)  (last use: the store)
func_80036940 80036AA8 lui at / 80036AAC sw v0,%lo(g_cdread_expected_pos)(at)  (store of CdPosToInt's v0; no load)
No slt/sltu, no sra/srl, no div/divu, no bltz/bgez/bgtz/blez, no divide fix-up, no lb/lh/lbu/lhu on the value.''')
