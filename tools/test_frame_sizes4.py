#!/usr/bin/env python3
"""Test: for outer (no if) + manual inversion inner variants"""
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

# S: for outer (no if) + manual inversion (if s7<*s4 + do-while) + newvar
variants['S_for_noif_manualinv_newvar'] = COMMON_TYPES + """
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
    var_s6 = 0x18;
    var_s5 = new_var;
    var_s3 = var_s4;
    var_s2 = (s16 *) (((u8 *) var_s4) + var_s5);
    if (var_s7 < (*((s32 *) var_s4))) {
      do {
""" + INNER_BODY + """\
      } while (var_s7 < (*((s32 *) var_s4)));
    }
    sp28 += 0xD0;
  }
}
"""

# T: same but setup INSIDE the if-guard (like target)
variants['T_for_noif_manualinv_setup_inside'] = COMMON_TYPES + """
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
    if (var_s7 < (*((s32 *) var_s4))) {
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

# U: same but with if(*s4 > 0) guard (standard guard, not manual inversion)
variants['U_for_noif_classic_guard'] = COMMON_TYPES + """
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

# V: for outer no if + for inner + newvar + if guard
variants['V_for_noif_for_inner_guard'] = COMMON_TYPES + """
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
"""

# W: for outer no if + while inner + newvar
variants['W_for_noif_while_inner'] = COMMON_TYPES + """
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
    var_s6 = 0x18;
    var_s5 = new_var;
    var_s3 = var_s4;
    var_s2 = (s16 *) (((u8 *) var_s4) + var_s5);
    while (var_s7 < (*((s32 *) var_s4))) {
""" + INNER_BODY + """\
    }
    sp28 += 0xD0;
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

    # Find stack offsets used
    stack_offsets = set()
    for line in asm.split('\n'):
        m2 = re.search(r'(?:lw|sw|lh|sh)\s+\$(\d+),(\d+)\(\$sp\)', line)
        if m2:
            reg, off = int(m2.group(1)), int(m2.group(2))
            if off < 48:
                stack_offsets.add(off)

    # Check inner register allocation
    s5_reg = s6_reg = s4_reg = s3_reg = s2_reg = s7_reg = '?'
    for line in asm.split('\n'):
        # s5 init: li $XX, 0x84
        if 'li' in line and ('0x00000084' in line or '132' in line):
            m2 = re.search(r'li\s+\$(\d+)', line)
            if m2: s5_reg = f'${m2.group(1)}'
        # s6 init: li $XX, 0x18
        if 'li' in line and ('0x00000018' in line or ',24' in line.replace(' ','')):
            m2 = re.search(r'li\s+\$(\d+)', line)
            if m2: s6_reg = f'${m2.group(1)}'

    target_s5 = '$21'  # s5
    target_s6 = '$22'  # s6
    s5_ok = '✓' if s5_reg == target_s5 else '✗'
    s6_ok = '✓' if s6_reg == target_s6 else '✗'
    frame_ok = '✓' if frame == 88 else '✗'
    blez_ok = '✓' if blez_count == 2 else '✗'

    print(f'{name}: frame={frame}{frame_ok} blez={blez_count}{blez_ok} vars={sorted(stack_offsets)} s5={s5_reg}{s5_ok} s6={s6_reg}{s6_ok}')
