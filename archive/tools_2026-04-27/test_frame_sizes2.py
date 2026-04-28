#!/usr/bin/env python3
"""Test more variants for frame=88 without extra instructions"""
import subprocess, re, sys

COMMON_TYPES = """\
typedef unsigned char u8;
typedef signed char s8;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned int u32;
typedef signed int s32;
extern void func_80052A20(s32 *, s32 *, s16 *);
"""

INNER_BODY_DOWHILE = """\
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

# G: for inner WITHOUT if-guard (let for-loop handle the guard)
variants['G_for_inner_no_guard'] = COMMON_TYPES + """
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
}
"""

# H: manual loop inversion - if(s7<*s4) then do-while
variants['H_manual_inversion'] = COMMON_TYPES + """
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
      var_s7 = 0;
      var_s4 = (s16 *) ((((u8 *) sp10) + sp28) + 0x1C);
      var_s6 = 0x18;
      var_s5 = new_var;
      var_s3 = var_s4;
      var_s2 = (s16 *) (((u8 *) var_s4) + var_s5);
      if (var_s7 < (*((s32 *) var_s4))) {
        do {
""" + INNER_BODY_DOWHILE + """\
        } while (var_s7 < (*((s32 *) var_s4)));
      }
      sp28 += 0xD0;
    }
  }
}
"""

# I: volatile sp10 (not a separate pad - sp10 itself is volatile)
variants['I_volatile_sp10'] = COMMON_TYPES + """
void func_8003F6D8(s16 *arg0) {
  volatile s16 *sp10;
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
    for (var_fp = 0; var_fp < (*(s16 *)sp10); var_fp++) {
      var_s7 = 0;
      var_s4 = (s16 *) ((((u8 *) sp10) + sp28) + 0x1C);
      if ((*((s32 *) var_s4)) > 0) {
        var_s6 = 0x18;
        var_s5 = new_var;
        var_s3 = var_s4;
        var_s2 = (s16 *) (((u8 *) var_s4) + var_s5);
        do {
""" + INNER_BODY_DOWHILE + """\
        } while (var_s7 < (*((s32 *) var_s4)));
      }
      sp28 += 0xD0;
    }
  }
}
"""

# J: goto-based inner loop
variants['J_goto_inner'] = COMMON_TYPES + """
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
      var_s7 = 0;
      var_s4 = (s16 *) ((((u8 *) sp10) + sp28) + 0x1C);
      if ((*((s32 *) var_s4)) > 0) {
        var_s6 = 0x18;
        var_s5 = new_var;
        var_s3 = var_s4;
        var_s2 = (s16 *) (((u8 *) var_s4) + var_s5);
        inner_top:
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
        if (var_s7 < (*((s32 *) var_s4))) goto inner_top;
      }
      sp28 += 0xD0;
    }
  }
}
"""

# K: while(1) + break inner
variants['K_while1_break'] = COMMON_TYPES + """
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
      var_s7 = 0;
      var_s4 = (s16 *) ((((u8 *) sp10) + sp28) + 0x1C);
      if ((*((s32 *) var_s4)) > 0) {
        var_s6 = 0x18;
        var_s5 = new_var;
        var_s3 = var_s4;
        var_s2 = (s16 *) (((u8 *) var_s4) + var_s5);
        while (1) {
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
          if (var_s7 >= (*((s32 *) var_s4))) break;
        }
      }
      sp28 += 0xD0;
    }
  }
}
"""

# L: do-while with setup OUTSIDE the if (unconditional setup)
variants['L_setup_outside_if'] = COMMON_TYPES + """
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
      var_s7 = 0;
      var_s4 = (s16 *) ((((u8 *) sp10) + sp28) + 0x1C);
      var_s6 = 0x18;
      var_s5 = new_var;
      var_s3 = var_s4;
      var_s2 = (s16 *) (((u8 *) var_s4) + var_s5);
      if ((*((s32 *) var_s4)) > 0) {
        do {
""" + INNER_BODY_DOWHILE + """\
        } while (var_s7 < (*((s32 *) var_s4)));
      }
      sp28 += 0xD0;
    }
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
    # Get frame info from .frame directive
    m = re.search(r'\.frame\s+\$sp,(\d+)', asm)
    frame = int(m.group(1)) if m else '???'
    # Count blez instructions
    blez_count = len(re.findall(r'\bblez\b', asm))
    # Get sp10 offset
    sp10_match = re.search(r'sw\s+\$4,(\d+)\(\$sp\)', asm)
    sp10_off = sp10_match.group(1) if sp10_match else '???'
    # Get sp28 offset (look for store of 8)
    sp28_matches = re.findall(r'sw\s+\$\d+,(\d+)\(\$sp\)', asm)
    print(f'{name}: frame={frame} blez={blez_count} sp10_off={sp10_off} stack_stores={sp28_matches[:6]}')
