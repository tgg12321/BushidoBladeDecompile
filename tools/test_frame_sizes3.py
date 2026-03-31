#!/usr/bin/env python3
"""Test variants: remove explicit if-wrapper, try different outer/inner loop combos"""
import subprocess, re

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

# M: for outer (NO if wrapper) + do-while inner + volatile pad
variants['M_for_noif_dowhile_volatile'] = COMMON_TYPES + """
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
"""

# N: for outer (NO if wrapper) + for inner (no if guard) + newvar
variants['N_for_noif_for_inner_noguard'] = COMMON_TYPES + """
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
  new_var = 0x84;
  sp28 = 8;
  for (var_fp = 0; var_fp < (*sp10); var_fp++) {
    var_s4 = (s16 *) ((((u8 *) sp10) + sp28) + 0x1C);
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
    sp28 += 0xD0;
  }
}
"""

# O: while outer (no if wrapper) + do-while inner + newvar
variants['O_while_outer_dowhile_inner'] = COMMON_TYPES + """
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
  new_var = 0x84;
  sp28 = 8;
  var_fp = 0;
  while (var_fp < (*sp10)) {
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
  }
}
"""

# P: for outer (no if wrapper) + do-while inner + newvar (no volatile)
variants['P_for_noif_dowhile_newvar'] = COMMON_TYPES + """
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
  new_var = 0x84;
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
"""

# Q: do-while outer (with if guard) + for inner + newvar
variants['Q_dowhile_outer_for_inner'] = COMMON_TYPES + """
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
    var_fp = 0;
    do {
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
      var_fp++;
    } while (var_fp < (*sp10));
  }
}
"""

# R: do-while outer + do-while inner + volatile (force frame)
variants['R_dowhile_both_volatile'] = COMMON_TYPES + """
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
    var_fp = 0;
    do {
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
    m = re.search(r'\.frame\s+\$sp,(\d+)', asm)
    frame = int(m.group(1)) if m else '???'
    blez_count = len(re.findall(r'\bblez\b', asm))
    sp10_match = re.search(r'sw\s+\$4,(\d+)\(\$sp\)', asm)
    sp10_off = int(sp10_match.group(1)) if sp10_match else '???'
    # Find sp28 offset (the store of value 8)
    sp28_off = '???'
    for line in asm.split('\n'):
        if 'sw' in line and '($sp)' in line:
            # Look for the store that follows the li 8
            pass
    # Find all stack variable accesses (not reg saves)
    stack_accesses = set()
    for line in asm.split('\n'):
        m2 = re.search(r'(?:lw|sw|lh|sh)\s+\$\d+,(\d+)\(\$sp\)', line)
        if m2:
            off = int(m2.group(1))
            if off < 48:  # below saved regs area
                stack_accesses.add(off)

    # Check register allocation by looking at inner loop
    inner_regs = {}
    for line in asm.split('\n'):
        if 'li' in line and '0x00000084' in line:
            m2 = re.search(r'li\s+\$(\d+)', line)
            if m2: inner_regs['s5_init'] = f'${m2.group(1)}'
        if 'li' in line and '0x00000018' in line:
            m2 = re.search(r'li\s+\$(\d+)', line)
            if m2: inner_regs['s6_init'] = f'${m2.group(1)}'

    print(f'{name}: frame={frame} blez={blez_count} sp10={sp10_off} vars={sorted(stack_accesses)} s5={inner_regs.get("s5_init","?")} s6={inner_regs.get("s6_init","?")}')
