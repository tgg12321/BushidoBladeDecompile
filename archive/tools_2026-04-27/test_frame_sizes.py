#!/usr/bin/env python3
"""Test different C variants to find which produces frame=88 (0x58)"""
import subprocess, re, sys, os

COMMON_TYPES = """\
typedef unsigned char u8;
typedef signed char s8;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned int u32;
typedef signed int s32;
extern void func_80052A20(s32 *, s32 *, s16 *);
"""

INNER_BODY = """\
          var_s1 = (s16 *) (((u8 *) var_s4) + var_s6);
          var_s6 += 0x20;
          var_s5 += 0x10;
          var_s0 = *((s32 *) (((u8 *) var_s3) + 4));
          var_s3 = (s16 *) (((u8 *) var_s3) + 4);
          var_s0 += 0x18;
          func_80052A20((s32 *) var_s0, (s32 *) var_s2, var_s1);
          func_80052A20((s32 *) var_s0, (s32 *) (((u8 *) var_s2) + 8), (s16 *) (((u8 *) var_s1) + 0x10));
          var_s7 += 1;
          var_s2 = (s16 *) (((u8 *) var_s4) + var_s5);
"""

variants = {}

# A: volatile pad
variants['A_volatile_pad'] = COMMON_TYPES + """
void func_8003F6D8(s16 *arg0) {
  s16 *sp10;
  s32 sp28;
  s32 var_s0;
  s16 *var_s1;
  s16 *var_s2;
  s16 *var_s3;
  s16 *var_s4;
  volatile s32 pad;
  s32 var_s5;
  s32 var_s6;
  s32 var_s7;
  s32 var_fp;
  sp10 = arg0;
  if ((*arg0) > 0) {
    pad = 0x84;
    sp28 = 8;
    for (var_fp = 0; var_fp < (*sp10); var_fp++) {
      var_s7 = 0;
      var_s4 = (s16 *) ((((u8 *) sp10) + sp28) + 0x1C);
      if ((*((s32 *) var_s4)) > 0) {
        var_s6 = 0x18;
        var_s5 = 0x84;
        var_s3 = var_s4;
        var_s2 = (s16 *) (((u8 *) var_s4) + var_s5);
        do {
""" + INNER_BODY + """\
        } while (var_s7 < (*((s32 *) var_s4)));
      }
      sp28 += 0xD0;
    }
  }
}
"""

# B: sp1C as third stack variable used to init var_s5
variants['B_sp1C_used'] = COMMON_TYPES + """
void func_8003F6D8(s16 *arg0) {
  s16 *sp10;
  s32 sp1C;
  s32 sp28;
  s32 var_s0;
  s16 *var_s1;
  s16 *var_s2;
  s16 *var_s3;
  s16 *var_s4;
  s32 var_s5;
  s32 var_s6;
  s32 var_s7;
  s32 var_fp;
  sp10 = arg0;
  if ((*arg0) > 0) {
    sp1C = 0x84;
    sp28 = 8;
    for (var_fp = 0; var_fp < (*sp10); var_fp++) {
      var_s7 = 0;
      var_s4 = (s16 *) ((((u8 *) sp10) + sp28) + 0x1C);
      if ((*((s32 *) var_s4)) > 0) {
        var_s6 = 0x18;
        var_s5 = sp1C;
        var_s3 = var_s4;
        var_s2 = (s16 *) (((u8 *) var_s4) + var_s5);
        do {
""" + INNER_BODY + """\
        } while (var_s7 < (*((s32 *) var_s4)));
      }
      sp28 += 0xD0;
    }
  }
}
"""

# C: two int new_vars
variants['C_two_newvars'] = COMMON_TYPES + """
void func_8003F6D8(s16 *arg0) {
  s16 *sp10;
  s32 sp28;
  s32 var_s0;
  s16 *var_s1;
  s16 *var_s2;
  s16 *var_s3;
  s16 *var_s4;
  int new_var;
  int new_var2;
  s32 var_s5;
  s32 var_s6;
  s32 var_s7;
  s32 var_fp;
  sp10 = arg0;
  if ((*arg0) > 0) {
    new_var = 0x84;
    new_var2 = 0x18;
    sp28 = 8;
    for (var_fp = 0; var_fp < (*sp10); var_fp++) {
      var_s7 = 0;
      var_s4 = (s16 *) ((((u8 *) sp10) + sp28) + 0x1C);
      if ((*((s32 *) var_s4)) > 0) {
        var_s6 = new_var2;
        var_s5 = new_var;
        var_s3 = var_s4;
        var_s2 = (s16 *) (((u8 *) var_s4) + var_s5);
        do {
""" + INNER_BODY + """\
        } while (var_s7 < (*((s32 *) var_s4)));
      }
      sp28 += 0xD0;
    }
  }
}
"""

# D: new_var = 0x84 as int, with extra s32 pad declared but unused
variants['D_newvar_plus_pad'] = COMMON_TYPES + """
void func_8003F6D8(s16 *arg0) {
  s16 *sp10;
  s32 sp28;
  s32 var_s0;
  s16 *var_s1;
  s16 *var_s2;
  s16 *var_s3;
  s16 *var_s4;
  int new_var;
  s32 pad;
  s32 var_s5;
  s32 var_s6;
  s32 var_s7;
  s32 var_fp;
  sp10 = arg0;
  if ((*arg0) > 0) {
    new_var = 0x84;
    pad = 0;
    sp28 = 8;
    for (var_fp = 0; var_fp < (*sp10); var_fp++) {
      var_s7 = 0;
      var_s4 = (s16 *) ((((u8 *) sp10) + sp28) + 0x1C);
      if ((*((s32 *) var_s4)) > 0) {
        var_s6 = 0x18;
        var_s5 = new_var;
        var_s3 = var_s4;
        var_s2 = (s16 *) (((u8 *) var_s4) + var_s5);
        do {
""" + INNER_BODY + """\
        } while (var_s7 < (*((s32 *) var_s4)));
      }
      sp28 += 0xD0;
    }
  }
}
"""

# E: for outer + for inner (known frame=88) but with new_var trick
variants['E_for_for_newvar'] = COMMON_TYPES + """
void func_8003F6D8(s16 *arg0) {
  s16 *sp10;
  s32 sp28;
  s32 var_s0;
  s16 *var_s1;
  s16 *var_s2;
  s16 *var_s3;
  s16 *var_s4;
  int new_var;
  s32 var_s5;
  s32 var_s6;
  s32 var_s7;
  s32 var_fp;
  sp10 = arg0;
  if ((*arg0) > 0) {
    new_var = 0x84;
    sp28 = 8;
    for (var_fp = 0; var_fp < (*sp10); var_fp++) {
      var_s4 = (s16 *) ((((u8 *) sp10) + sp28) + 0x1C);
      if ((*((s32 *) var_s4)) > 0) {
        var_s6 = 0x18;
        var_s5 = new_var;
        var_s3 = var_s4;
        var_s2 = (s16 *) (((u8 *) var_s4) + var_s5);
        for (var_s7 = 0; var_s7 < (*((s32 *) var_s4)); var_s7++) {
          var_s1 = (s16 *) (((u8 *) var_s4) + var_s6);
          var_s6 += 0x20;
          var_s5 += 0x10;
          var_s0 = *((s32 *) (((u8 *) var_s3) + 4));
          var_s3 = (s16 *) (((u8 *) var_s3) + 4);
          var_s0 += 0x18;
          func_80052A20((s32 *) var_s0, (s32 *) var_s2, var_s1);
          func_80052A20((s32 *) var_s0, (s32 *) (((u8 *) var_s2) + 8), (s16 *) (((u8 *) var_s1) + 0x10));
          var_s2 = (s16 *) (((u8 *) var_s4) + var_s5);
        }
      }
      sp28 += 0xD0;
    }
  }
}
"""

# F: do-while outer + do-while inner + extra declared vars
variants['F_dowhile_both_extra'] = COMMON_TYPES + """
void func_8003F6D8(s16 *arg0) {
  s16 *sp10;
  s32 sp28;
  s32 var_s0;
  s16 *var_s1;
  s16 *var_s2;
  s16 *var_s3;
  s16 *var_s4;
  int new_var;
  s32 extra1;
  s32 extra2;
  s32 var_s5;
  s32 var_s6;
  s32 var_s7;
  s32 var_fp;
  sp10 = arg0;
  if ((*arg0) > 0) {
    new_var = 0x84;
    extra1 = 0;
    extra2 = 0;
    sp28 = 8;
    var_fp = 0;
    do {
      var_s7 = 0;
      var_s4 = (s16 *) ((((u8 *) sp10) + sp28) + 0x1C);
      if ((*((s32 *) var_s4)) > 0) {
        var_s6 = 0x18;
        var_s5 = new_var;
        var_s3 = var_s4;
        var_s2 = (s16 *) (((u8 *) var_s4) + var_s5);
        do {
""" + INNER_BODY + """\
        } while (var_s7 < (*((s32 *) var_s4)));
      }
      sp28 += 0xD0;
      var_fp++;
    } while (var_fp < (*sp10));
  }
}
"""

CC = 'tools/gcc-2.7.2/cc1'

for name, src in sorted(variants.items()):
    tmpfile = f'/tmp/test_{name}.c'
    outfile = f'/tmp/test_{name}.s'
    with open(tmpfile, 'w') as f:
        f.write(src)
    r = subprocess.run([CC, '-O2', '-G0', '-mips1', '-mcpu=3000', '-quiet', tmpfile, '-o', outfile],
                       capture_output=True, text=True)
    if r.returncode != 0:
        print(f'{name}: COMPILE ERROR')
        for line in r.stderr.strip().split('\n')[:3]:
            print(f'  {line}')
        continue
    with open(outfile) as f:
        asm = f.read()
    # Find frame size
    m = re.search(r'addiu\s+\$sp,\$sp,(-\d+)', asm)
    frame = abs(int(m.group(1))) if m else '???'
    # Count saved regs
    saves = len(re.findall(r'\bsw\s+\$(?:s\d|fp|ra),', asm))
    # Check for inner blez (extra branch from for-loop)
    inner_blez = len(re.findall(r'blez.*\.L', asm))
    print(f'{name}: frame={frame} saved_regs={saves} inner_blez_count={inner_blez}')
