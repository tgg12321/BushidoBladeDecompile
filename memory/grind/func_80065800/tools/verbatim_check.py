"""verbatim_check.py (repo root, after move.py): confirm the boundary move is verbatim.
1. new text1b.c == HEAD text1b.c + the HEAD text1b_tu1c.c block (snd_Init's extern block ..
   func_80060E38) minus the two typedefs text1b.c already has from include/gte.h;
2. new text1b_tu1c.c, after its rebuilt header, == HEAD text1b_tu1c.c from func_80061064's extern
   block on, except that `extern s32 D_800158E0;` became the moved D_800158E0 definition and the
   two moved jtbl definitions sit before func_80065800's INCLUDE_ASM;
3. those moved definitions are byte-identical to the text removed from text1a_b_pre_rodata_b.c."""
import subprocess

NL = chr(10)


def head(p):
    return subprocess.run(['git', 'show', 'HEAD:' + p], capture_output=True, check=True).stdout.decode('utf-8')


def cur(p):
    return open(p, 'rb').read().decode('utf-8')


hb, ht, hr = head('src/text1b.c'), head('src/text1b_tu1c.c'), head('src/text1a_b_pre_rodata_b.c')
nb, nt, nr = cur('src/text1b.c'), cur('src/text1b_tu1c.c'), cur('src/text1a_b_pre_rodata_b.c')
a = ht.index('\nextern s16 D_800A3400;\n') + 1
b = ht.index('extern s32 func_80041E10();\nextern s32 func_800421A4();\n')
block = ht[a:b]
for d in ['typedef struct CVECTOR { u8 r, g, b, cd; } CVECTOR;\n',
          'typedef struct DVECTOR { s16 vx, vy; } DVECTOR;\n']:
    assert block.count(d) == 1
    block = block.replace(d, '')
print('1 text1b.c = HEAD + moved block:', nb == hb + block)

defn_start = hr.index('/* D_800158E0:')
defn = hr[defn_start:hr.index('\n', hr.index('const char D_800158E0', defn_start)) + 1]
tab = hr[hr.index('/* jtbl_800158F8:'):hr.index('/* NOTE: the cluster continues')]
old_tail = ht[b:]
exp = old_tail.replace('\nextern s32 D_800158E0;\n', '\n' + defn, 1)
k = exp.index('INCLUDE_ASM("asm/funcs", func_80065800);\n')
exp = exp[:k] + tab + exp[k:]
print('2 text1b_tu1c.c body = HEAD tail with the two relocations:', nt.endswith(exp))
print('3 moved definitions removed from text1a_b_pre_rodata_b.c:', defn not in nr and tab not in nr
      and ('D_800158B4' in nr and 'D_800158CC' in nr))
print('   new text1b_tu1c.c header lines:', nt[:len(nt) - len(exp)].count(NL))
