"""-G8 prong (iv) record: the five TUs are the pre-split file cut at four lines.
usage (WSL, repo root): python3 tmp/func_80036140/movecheck.py <final-tree> <inplace-tree> <head-rev>
  final-tree  : the landing (apply_model --split final --merge --ext rec)
  inplace-tree: the same respellings with NO split (apply_model --split none --merge --ext rec)
Prints (1) the in-place respelling diff vs <head-rev> (what is NOT a move) and (2) for every new file,
that its text after its header block is a contiguous, byte-identical slice of the in-place file."""
import difflib, subprocess, sys
from pathlib import Path

F, I, rev = Path(sys.argv[1]), Path(sys.argv[2]), sys.argv[3]
H = Path('tmp/func_80036140')
src = 'src/code6cac_b2_post.c'
head = subprocess.run(['git', 'show', f'{rev}:{src}'], capture_output=True, text=True, check=True).stdout
inp = (I / src).read_text()
print(f'== (1) respellings in place ({src} @ {rev} -> converted, unsplit):')
for l in difflib.unified_diff(head.split('\n'), inp.split('\n'), 'HEAD', 'inplace', n=0, lineterm=''):
    print('   ', l)
L = inp.split('\n')
def at(line):
    assert L.count(line) == 1, line
    return L.index(line)
i_mix = at('extern void CdMix(CdlATV *);')
i_snd = at('void snd_SerialMixOn(void) {')
i_jt = at("/* func_80036140's jump table (func_80036140 is still INCLUDE_ASM, so the table is")
i_inc = at('INCLUDE_ASM("asm/funcs", func_80036140);')
i_idle = at('s32 cdrom_IsIdle(void) {')
body = (H / 'body_tpl.c').read_text()
for k, v in {'@E9C_POSTINC@': 'D_80101E58.rec.unk3C++', '@E9C@': 'D_80101E58.rec.unk3C',
             '@EA4_SUB4@': 'D_80101E58.rec.unk44 -= 4', '@EA4@': 'D_80101E58.rec.unk44'}.items():
    body = body.replace(k, v)
checks = [
    ('src/code6cac_b2_post.c', '', L[:i_mix], None),
    ('src/code6cac_b4.c', (H / 'b4_head.txt').read_text(), L[i_mix:i_snd], None),
    ('src/code6cac_b4_post.c', (H / 'b4_post_head.txt').read_text(), L[i_snd:i_jt], None),
    ('src/code6cac_b5.c', (H / 'b5_head.txt').read_text() + body, L[i_inc + 1:i_idle], None),
    ('src/code6cac_b5_post.c', (H / 'b5_post_head.txt').read_text(), L[i_idle:], None),
]
print('\n== (2) the split')
ok = True
for rel, hdr, sl, _ in checks:
    got = (F / rel).read_text()
    want = hdr + '\n'.join(sl) + ('' if rel.endswith('b5_post.c') else '\n')
    same = got == want
    ok &= same
    print(f'  {rel}: header {hdr.count(chr(10))} lines + moved slice of {len(sl)} lines '
          f'(in-place lines {L.index(sl[0]) + 1 if sl else 0}..{L.index(sl[0]) + len(sl) if sl else 0}) '
          f'-> {"IDENTICAL" if same else "DIFFERS"}')
    if not same:
        for l in list(difflib.unified_diff(want.split('\n'), got.split('\n'), 'want', 'got', n=1, lineterm=''))[:20]:
            print('     ', l)
print(f'  dropped from the in-place file: lines {i_jt + 1}..{i_inc} (the transcribed jtbl_80010938 comment + '
      f'array, now emitted by the compiler) and line {i_inc + 1} (INCLUDE_ASM, replaced by the C body)')
print('RESULT:', 'every moved line byte-identical' if ok else 'MISMATCH')
