"""Calibration only (-G8 prong (ii) / maspsx_comm_syms prong (a)): the ORIGINAL toolchain, PsyQ cc1psx ->
Sony ASPSX 2.34, on a scratch tree's TU, with chosen symbols re-declared as the original file's storage
class (tentative definition / static / initialized / extern), at -G8 and -G0. Prints, per function, the
word count and differing words vs the shipped asm/funcs bytes (relocation immediates masked) and the
list of gp-relative accesses in the assembled object.
usage (WSL, repo root): python3 tmp/func_80036140/calib.py <tree> <stem> <tag> <class> <G> func... -- sym...
  class: comm | static | init | extern"""
import re, subprocess, sys
from pathlib import Path
sys.path.insert(0, 'tools/maspsx/aspsx')
sys.path.insert(0, 'tmp/func_80036140/r')
sys.path.insert(0, 'tmp/func_80036140')
import util
from run_asm_tests import decode

a = sys.argv[1:]
tree, stem, tag, cls, G = a[:5]
k = a.index('--')
funcs, syms = a[5:k], a[k + 1:]
O = Path('tmp/func_80036140/calib') / tag
O.mkdir(parents=True, exist_ok=True)
T = Path('tmp/func_80036140') / tree
cpp = ['mipsel-linux-gnu-cpp', f'-I{T}/include', '-undef', '-Wall', '-lang-c', '-fno-builtin', '-Dmips',
       '-D__GNUC__=2', '-D__OPTIMIZE__', '-D__mips__', '-D__mips', '-Dpsx', '-D__psx__', '-D__psx', '-D_PSYQ',
       '-D__EXTENSIONS__', '-D_MIPSEL', '-D_LANGUAGE_C', '-DLANGUAGE_C', f'{T}/src/{stem}.c']
i = subprocess.run(cpp, capture_output=True, text=True).stdout
# re-declare each listed symbol: every `extern <type> sym;` line for it becomes the chosen storage class
for s in syms:
    pat = re.compile(r'^extern\s+([^;()]*?)\b' + re.escape(s) + r'\s*(\[[^\]]*\])?\s*;\s*$', re.M)
    hits = pat.findall(i)
    assert hits, s
    first = [True]
    def sub(m):
        t = m.group(1)
        a = m.group(2) or ''
        if not first[0]:
            return ''                       # drop duplicate declarations
        first[0] = False
        return {'comm': f'{t}{s}{a};', 'static': f'static {t}{s}{a};', 'extern': f'extern {t}{s}{a};',
                'init': f'{t}{s}{a} = {{0}};' if ('CdlATV' in t or a) else f'{t}{s} = 1;'}[cls]
    i = pat.sub(sub, i)
(O / f'{stem}.i').write_text(i)
s_path = O / f'{stem}{G}.s'
with open(O / f'{stem}.i') as fi, open(s_path, 'w') as fo:
    if tag.startswith('OURS'):
        subprocess.run(['tools/gcc-2.7.2/build/cc1', '-O2', G, '-funsigned-char', '-quiet', '-mcpu=3000', '-mips1',
                        '-mno-abicalls', '-fno-builtin', '-w', '-mel', '-msoft-float'], stdin=fi, stdout=fo, check=True)
    else:
        subprocess.run(['bash', 'tools/cc1psx_wrapper.sh', '-O2', G, '-funsigned-char', '-quiet', '-mcpu=3000',
                        '-mips1', '-msoft-float', '-w'], stdin=fi, stdout=fo, check=True)
if tag.startswith('OURS'):                     # ELF-flavoured directives ASPSX does not know
    L = []
    for line in s_path.read_text().split('\n'):
        w = line.split()
        if w and w[0] in ('.version', '.size', '.type', '.ident'):
            continue
        if w and w[0] == '.comm' and w[1].count(',') == 2:
            line = '\t.comm\t' + w[1].rsplit(',', 1)[0]
        L.append(line)
    s_path.write_text('\n'.join(L))
o_path = O / f'{stem}{G}.obj'
r = subprocess.run(['bash', 'tmp/func_80036140/r/aspsx.sh', str(s_path), str(o_path), '-G8'],
                   capture_output=True, text=True)
if not o_path.exists():
    print('ASPSX failed', r.stderr[-800:]); sys.exit(1)
import lnk
t = lnk.text_bytes(o_path.read_bytes())
ours = [int.from_bytes(t[j:j + 4], 'little') for j in range(0, len(t), 4)]


def mask(w):
    op = w >> 26
    if op in (2, 3):
        return w & 0xFC000000
    if op in (0x0F, 0x09) or op >= 0x20:
        return w & 0xFFFF0000
    return w


tgt = []
for f in funcs:
    words = [int(m.group(1)[6:8] + m.group(1)[4:6] + m.group(1)[2:4] + m.group(1)[0:2], 16)
             for m in (re.search(r'/\*\s*[0-9A-F]+\s+[0-9A-F]{8}\s+([0-9A-F]{8})\s*\*/', l)
                       for l in open(f'asm/funcs/{f}.s')) if m]
    tgt.append((f, words))
pos = 0
out = []
for f, words in tgt:
    so = lnk.SYMS.get(f, (None, None))[1]
    ends = sorted(o for sec, o in lnk.SYMS.values() if sec == '.text' and so is not None and o > so)
    seg = ours[so // 4:(ends[0] // 4 if ends else len(ours))] if so is not None else ours[pos:pos + len(words)]
    import difflib
    sm = difflib.SequenceMatcher(None, [mask(x) for x in words], [mask(y) for y in seg], autojunk=False)
    bad = max(len(words), len(seg)) - sum(b.size for b in sm.get_matching_blocks())
    gp_t = sum(1 for x in words if (x >> 26) >= 0x20 and ((x >> 21) & 31) == 28)
    gp_o = sum(1 for x in seg if (x >> 26) >= 0x20 and ((x >> 21) & 31) == 28)
    base = so if so is not None else pos * 4
    gp_os = ['+'.join(lnk.RELOCS.get(base + 4 * j, ['?'])) for j, x in enumerate(seg) if (x >> 26) >= 0x20 and ((x >> 21) & 31) == 28]
    gp_ts = re.findall(r'%gp_rel\(([A-Za-z0-9_+]+)\)', open(f'asm/funcs/{f}.s').read())
    out.append(f'{f}: {len(words)} target words, {bad} differ after alignment (reloc imm masked); gp accesses target={gp_t} ours={gp_o}')
    out.append(f'    target gp: {gp_ts}')
    out.append(f'    ours   gp: {gp_os}')
    pos += len(words)
out.append(f'total words ours={len(ours)} target={sum(len(w) for _, w in tgt)}')
res = f'[{tag}] {stem} class={cls} cc1psx {G} syms={",".join(syms)}\n' + '\n'.join('  ' + x for x in out)
print(res)
(O / f'{stem}{G}.result.txt').write_text(res + '\n')
