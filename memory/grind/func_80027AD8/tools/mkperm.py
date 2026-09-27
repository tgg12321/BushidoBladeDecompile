"""Build a decomp-permuter workspace for func_80027AD8 at tmp/perm_7ad8/<label>.
usage: python3 mkperm.py <candidate.c> <label>"""
import os, re, sys, shutil, subprocess
root = '/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile'
cand, label = sys.argv[1], sys.argv[2]
ws = f'{root}/tmp/perm_7ad8/{label}'
os.makedirs(ws, exist_ok=True)
ctx = '''typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;
typedef unsigned short u16;
typedef signed int s32;
typedef unsigned int u32;
typedef struct {
    s16 unk0;
    s16 unk2;
    s16 unk4;
    s16 unk6;
    u16 unk8;
    s16 unkA;
    u8 unkC;
    u8 unkD;
} Tbl8008E194;
extern s16 D_800A37E8;
extern s16 D_800A37EA;
extern s16 D_800A37EC;
extern s16 D_800A38DC;
extern s32 ratan2(s32, s32);
extern void func_8001F860(s16 *arg0, s32 arg1);
extern void func_80032854(s32, s32, u8 *, s16 *);
s32 func_800272FC(s32 a0);
void func_8002738C(s32 a0, s32 a1);
void func_80027438(u8 *a0, s32 a1, s16 a2);
void func_80027640(s32 arg0);
void func_800278C0(s32 a0, s32 *ptr, s32 cmd, s32 a3, u8 *stack_a2, s32 stack_v1);
void func_80027A58(s32 *a0);
'''
body = open(cand).read()
open(f'{ws}/base.c', 'w').write(ctx + body)
# target
pre = open(f'{root}/tmp/perm_a880/target.s').read()
cut = pre.index('glabel func_8006A880') if 'glabel func_8006A880' in pre else None
prelude = pre[:cut] if cut else pre
asm = open(f'{root}/asm/funcs/func_80027AD8.s').read()
jt = open(f'{root}/asm/rodata/jtbl_80010548.s').read()
tgt = prelude + '\n.section .text\n' + asm + '\n.section .rodata\n' + jt + '\n'
open(f'{ws}/target.s', 'w').write(tgt)
r = subprocess.run(['mipsel-linux-gnu-as', '-I', f'{root}/include', '-march=r3000', '-mtune=r3000',
                    '-no-pad-sections', '-O1', '-G0', '-o', f'{ws}/target.o', f'{ws}/target.s'],
                   capture_output=True, text=True)
print('as:', r.returncode, r.stderr[-2000:])
shutil.copy(f'{root}/tmp/perm_a880/compile.sh', f'{ws}/compile.sh')
open(f'{ws}/settings.toml', 'w').write('func_name = "func_80027AD8"\ncompiler_type = "gcc"\n')
print(ws)
