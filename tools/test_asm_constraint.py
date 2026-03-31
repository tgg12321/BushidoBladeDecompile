#!/usr/bin/env python3
"""Test asm constraints to prevent constant propagation while keeping register alloc"""
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

# Base template - all use literal 0x84 and if+for wrapper
# {ASM_TRICK} is placed between var_s5 init and var_s2 computation
TEMPLATE = COMMON_TYPES + """
void func_8003F6D8(s16 *arg0) {{
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
  if ((*arg0) > 0) {{
    sp28 = 8;
    for (var_fp = 0; var_fp < (*sp10); var_fp++) {{
      var_s7 = 0;
      var_s4 = (s16 *) ((((u8 *) sp10) + sp28) + 0x1C);
      if ((*((s32 *) var_s4)) > 0) {{
        var_s6 = 0x18;
        var_s5 = 0x84;
        var_s3 = var_s4;
{ASM_TRICK}
        var_s2 = (s16 *) (((u8 *) var_s4) + var_s5);
        do {{
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
        }} while (var_s7 < (*((s32 *) var_s4)));
      }}
      sp28 += 0xD0;
    }}
  }}
}}
"""

tricks = {
    "none":              "",
    "asm_empty":         '        __asm__("");',
    "asm_output_s5":     '        __asm__("" : "=r"(var_s5) : "0"(var_s5));',
    "asm_input_s5":      '        __asm__("" : : "r"(var_s5));',
    "asm_volatile":      '        __asm__ volatile("");',
    "asm_clobber_s5":    '        __asm__("" : "+r"(var_s5));',
    "asm_clobber_s4":    '        __asm__("" : "+r"(var_s4));',
    "asm_clobber_both":  '        __asm__("" : "+r"(var_s5), "+r"(var_s4));',
    "asm_clobber_s3s4s5":'        __asm__("" : "+r"(var_s3), "+r"(var_s4), "+r"(var_s5));',
}

CC = 'tools/gcc-2.7.2/cc1'
target_regs = {'s3': 19, 's4': 20, 's5': 21, 's6': 22, 's7': 23, 's2': 18}

for name, trick in tricks.items():
    src = TEMPLATE.format(ASM_TRICK=trick)
    tmpfile = f'/tmp/test_asm_{name}.c'
    outfile = f'/tmp/test_asm_{name}.s'
    with open(tmpfile, 'w') as f:
        f.write(src)
    r = subprocess.run([CC, '-O2', '-G0', '-mips1', '-mcpu=3000', '-quiet', tmpfile, '-o', outfile],
                       capture_output=True, text=True)
    if r.returncode != 0:
        print(f'{name:25s}: COMPILE ERROR: {r.stderr[:100]}')
        continue
    with open(outfile) as f:
        asm = f.read()

    m = re.search(r'\.frame\s+\$sp,(\d+)', asm)
    frame = int(m.group(1)) if m else '???'
    blez_count = len(re.findall(r'\bblez\b', asm))
    const_fold = any(',160' in line.replace(' ', '') or ',0xa0' in line.lower().replace(' ', '') for line in asm.split('\n') if 'addu' in line)

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
    print(f'{name:25s}: frame={frame} blez={blez_count} regs={correct}/{len(reg_map)} cf={cf} | {detail}')
