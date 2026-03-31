#!/usr/bin/env python3
"""Test: split new_var usage to keep register allocation but prevent constant folding"""
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

variants = {}

# Y1: var_s5 = 0x84 literal, but use new_var for initial s2 computation only
variants['Y1_split_newvar'] = COMMON_TYPES + """
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
        var_s5 = 0x84;
        var_s3 = var_s4;
        var_s2 = (s16 *) (((u8 *) var_s4) + new_var);
        do {
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
        } while (var_s7 < (*((s32 *) var_s4)));
      }
      sp28 += 0xD0;
    }
  }
}
"""

# Y2: same but with no if wrapper (for outer only)
variants['Y2_split_newvar_noif'] = COMMON_TYPES + """
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
      var_s5 = 0x84;
      var_s3 = var_s4;
      var_s2 = (s16 *) (((u8 *) var_s4) + new_var);
      do {
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
      } while (var_s7 < (*((s32 *) var_s4)));
    }
    sp28 += 0xD0;
  }
}
"""

# Y3: no new_var at all (pure X6 but no if wrapper) - check regs + constant folding
variants['Y3_literal_noif'] = COMMON_TYPES + """
void func_8003F6D8(s16 *arg0) {
  s16 *sp10;
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
      } while (var_s7 < (*((s32 *) var_s4)));
    }
    sp28 += 0xD0;
  }
}
"""

# Y4: asm barrier between s5=0x84 and s3=s4 to prevent folding of s2=s4+s5
variants['Y4_literal_asm_barrier'] = COMMON_TYPES + """
void func_8003F6D8(s16 *arg0) {
  s16 *sp10;
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
    sp28 = 8;
    for (var_fp = 0; var_fp < (*sp10); var_fp++) {
      var_s7 = 0;
      var_s4 = (s16 *) ((((u8 *) sp10) + sp28) + 0x1C);
      if ((*((s32 *) var_s4)) > 0) {
        var_s6 = 0x18;
        var_s5 = 0x84;
        var_s3 = var_s4;
        __asm__("");
        var_s2 = (s16 *) (((u8 *) var_s4) + var_s5);
        do {
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
        } while (var_s7 < (*((s32 *) var_s4)));
      }
      sp28 += 0xD0;
    }
  }
}
"""

# Y5: use cast to prevent constant folding: var_s2 = s4 + (s32)var_s5
variants['Y5_literal_cast'] = COMMON_TYPES + """
void func_8003F6D8(s16 *arg0) {
  s16 *sp10;
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
    sp28 = 8;
    for (var_fp = 0; var_fp < (*sp10); var_fp++) {
      var_s7 = 0;
      var_s4 = (s16 *) ((((u8 *) sp10) + sp28) + 0x1C);
      if ((*((s32 *) var_s4)) > 0) {
        var_s6 = 0x18;
        var_s5 = 0x84;
        var_s3 = var_s4;
        var_s2 = (s16 *) (((u8 *) var_s3) + var_s5);
        do {
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
        } while (var_s7 < (*((s32 *) var_s4)));
      }
      sp28 += 0xD0;
    }
  }
}
"""

# Y6: X1/X2 approach (asm barrier) BUT with literal (no new_var)
# asm before setup + if wrapper
variants['Y6_asm_literal_if'] = COMMON_TYPES + """
void func_8003F6D8(s16 *arg0) {
  s16 *sp10;
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
    sp28 = 8;
    for (var_fp = 0; var_fp < (*sp10); var_fp++) {
      var_s7 = 0;
      var_s4 = (s16 *) ((((u8 *) sp10) + sp28) + 0x1C);
      if ((*((s32 *) var_s4)) > 0) {
        __asm__("");
        var_s6 = 0x18;
        var_s5 = 0x84;
        var_s3 = var_s4;
        var_s2 = (s16 *) (((u8 *) var_s4) + var_s5);
        do {
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
        } while (var_s7 < (*((s32 *) var_s4)));
      }
      sp28 += 0xD0;
    }
  }
}
"""

CC = 'tools/gcc-2.7.2/cc1'
target_regs = {'s3': 19, 's4': 20, 's5': 21, 's6': 22, 's7': 23, 's2': 18}

for name, src in sorted(variants.items()):
    tmpfile = f'/tmp/test_{name}.c'
    outfile = f'/tmp/test_{name}.s'
    with open(tmpfile, 'w') as f:
        f.write(src)
    r = subprocess.run([CC, '-O2', '-G0', '-mips1', '-mcpu=3000', '-quiet', tmpfile, '-o', outfile],
                       capture_output=True, text=True)
    if r.returncode != 0:
        print(f'{name}: COMPILE ERROR: {r.stderr[:100]}')
        continue
    with open(outfile) as f:
        asm = f.read()

    m = re.search(r'\.frame\s+\$sp,(\d+)', asm)
    frame = int(m.group(1)) if m else '???'
    blez_count = len(re.findall(r'\bblez\b', asm))

    # Check for constant folding: look for addu $XX, $YY, 132 or addu $XX, $YY, 160
    const_fold = 'addu' in asm and ',160' in asm.replace(' ', '')

    reg_map = {}
    lines = asm.split('\n')
    for line in lines:
        m2 = re.search(r'li\s+\$(\d+),0x00000018', line)
        if m2: reg_map['s6'] = int(m2.group(1))
        m2 = re.search(r'li\s+\$(\d+),0x00000084', line)
        if m2: reg_map['s5'] = int(m2.group(1))
        m2 = re.search(r'addu\s+\$(\d+),\$\d+,28\s*$', line)
        if m2:
            c = int(m2.group(1))
            if c >= 16: reg_map['s4'] = c

    s4_r = reg_map.get('s4')
    if s4_r:
        for line in lines:
            m2 = re.match(r'\s+move\s+\$(\d+),\$' + str(s4_r) + r'\s*$', line)
            if m2:
                c = int(m2.group(1))
                if c >= 16 and c != s4_r:
                    reg_map['s3'] = c
                    break

    for line in lines:
        m2 = re.match(r'\s+addu\s+\$(\d+),\$\1,1\s*$', line)
        if m2:
            c = int(m2.group(1))
            if c >= 16: reg_map['s7'] = c

    s5_r = reg_map.get('s5')
    if s4_r and s5_r:
        for line in lines:
            m2 = re.search(r'addu\s+\$(\d+),\$' + str(s4_r) + r',\$' + str(s5_r), line)
            if m2:
                reg_map['s2'] = int(m2.group(1))
                break

    correct = sum(1 for k in target_regs if reg_map.get(k) == target_regs[k])
    detail = " ".join(f"{k}=${v}" + ("*" if target_regs.get(k)==v else "") for k, v in sorted(reg_map.items()))

    cf = "FOLD" if const_fold else "ok"
    ok_frame = '*' if frame == 88 else ''
    ok_blez = '*' if blez_count == 2 else ''
    print(f'{name:30s}: frame={frame}{ok_frame:1s} blez={blez_count}{ok_blez:1s} regs={correct}/{len(reg_map)} cf={cf} | {detail}')
