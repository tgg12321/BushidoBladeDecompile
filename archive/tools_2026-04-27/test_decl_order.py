#!/usr/bin/env python3
"""Test declaration order permutations for s3/s4/s5 register allocation"""
import subprocess, re, itertools

COMMON_TYPES = """\
typedef unsigned char u8;
typedef signed char s8;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned int u32;
typedef signed int s32;
extern void func_80052A20(s32 *, s32 *, s16 *);
"""

# Template with {DECLS} placeholder for variable declarations
TEMPLATE = COMMON_TYPES + """
void func_8003F6D8(s16 *arg0) {{
  s16 *sp10;
  s32 sp28;
{DECLS}
  sp10 = arg0;
  new_var = 0x84;
  sp28 = 8;
  for (var_fp = 0; var_fp < (*sp10); var_fp++) {{
    var_s7 = 0;
    var_s4 = (s16 *) ((((u8 *) sp10) + sp28) + 0x1C);
    if (var_s7 < (*((s32 *) var_s4))) {{
      var_s6 = 0x18;
      var_s5 = new_var;
      var_s3 = var_s4;
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
"""

# Variables that need to be declared - ordered groups
# Group A: these always need to be declared
fixed_before = []  # at top after sp10/sp28
fixed_after = ["  s32 var_fp;"]  # at bottom

# The variables we want to permute
permutable_vars = [
    "  s32 var_s0;",
    "  s16 *var_s1;",
    "  s16 *var_s2;",
    "  s16 *var_s3;",
    "  s16 *var_s4;",
    "  int new_var;",
    "  s32 var_s5;",
    "  s32 var_s6;",
    "  s32 var_s7;",
]

# We'll try specific reorderings focused on the s3/s4/s5 relationship
# Current order: s0, s1, s2, s3, s4, new_var, s5, s6, s7
# The target needs s3→$19, s4→$20, s5→$21

# Strategy: try moving new_var to different positions relative to s3/s4/s5
orderings = {
    "orig":     ["s0", "s1", "s2", "s3", "s4", "nv", "s5", "s6", "s7"],
    "nv_first": ["nv", "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7"],
    "nv_last":  ["s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", "nv"],
    "nv_after_s5": ["s0", "s1", "s2", "s3", "s4", "s5", "nv", "s6", "s7"],
    "nv_before_s3": ["s0", "s1", "s2", "nv", "s3", "s4", "s5", "s6", "s7"],
    "nv_after_s6": ["s0", "s1", "s2", "s3", "s4", "s5", "s6", "nv", "s7"],
    "nv_before_s0": ["nv", "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7"],
    "swap_s3s5": ["s0", "s1", "s2", "s5", "s4", "nv", "s3", "s6", "s7"],
    "s5_first": ["s5", "s0", "s1", "s2", "s3", "s4", "nv", "s6", "s7"],
    "s345_rev": ["s0", "s1", "s2", "s5", "s4", "s3", "nv", "s6", "s7"],
    "shift345": ["s0", "s1", "s2", "s4", "s5", "s3", "nv", "s6", "s7"],
    "s5s3s4":   ["s0", "s1", "s2", "s5", "s3", "s4", "nv", "s6", "s7"],
    "nv_s345":  ["s0", "s1", "s2", "nv", "s5", "s4", "s3", "s6", "s7"],
}

var_map = {
    "s0": "  s32 var_s0;",
    "s1": "  s16 *var_s1;",
    "s2": "  s16 *var_s2;",
    "s3": "  s16 *var_s3;",
    "s4": "  s16 *var_s4;",
    "s5": "  s32 var_s5;",
    "s6": "  s32 var_s6;",
    "s7": "  s32 var_s7;",
    "nv": "  int new_var;",
}

CC = 'tools/gcc-2.7.2/cc1'

# Target register assignments
# $19=s3→var_s3, $20=s4→var_s4, $21=s5→var_s5, $22=s6→var_s6, $23=s7→var_s7

for name, order in orderings.items():
    decls = "\n".join([var_map[v] for v in order]) + "\n" + fixed_after[0]
    src = TEMPLATE.format(DECLS=decls)

    tmpfile = f'/tmp/test_decl_{name}.c'
    outfile = f'/tmp/test_decl_{name}.s'
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

    # Extract register assignments by looking at the inner loop setup
    # Look for: li $XX, 0x18 (s6 init) and li $XX, 0x84 (s5 init)
    # And move $XX, $YY for s3=s4 copy
    # And addu $XX, $YY, $ZZ for s4 = base+0x1C

    reg_map = {}
    lines = asm.split('\n')
    for i, line in enumerate(lines):
        # s6 init: li $XX, 0x18
        m2 = re.search(r'li\s+\$(\d+),0x00000018', line)
        if m2: reg_map['s6'] = int(m2.group(1))
        # s5 init: li $XX, 0x84
        m2 = re.search(r'li\s+\$(\d+),0x00000084', line)
        if m2: reg_map['s5'] = int(m2.group(1))

    # Find the move that copies s4 to s3 (move $XX, $YY where YY is s4)
    for i, line in enumerate(lines):
        if 'addu' in line and ',28' in line and '$sp' not in line:
            # s4 = base + 28 → addu $XX, $YY, 28
            m2 = re.search(r'addu\s+\$(\d+)', line)
            if m2: reg_map['s4'] = int(m2.group(1))

    # Find s3 (the one that copies s4)
    s4_reg = reg_map.get('s4', None)
    if s4_reg:
        for line in lines:
            m2 = re.match(r'\s+move\s+\$(\d+),\$' + str(s4_reg), line)
            if m2:
                candidate = int(m2.group(1))
                if candidate != s4_reg and candidate >= 16:
                    reg_map['s3'] = candidate
                    break

    # s7 counter: the one incremented by 1 and compared
    for line in lines:
        m2 = re.match(r'\s+addu\s+\$(\d+),\$\1,1', line)
        if m2:
            candidate = int(m2.group(1))
            if candidate >= 16:
                reg_map['s7'] = candidate

    # s2: addu $XX, $s4, $s5
    s5_r = reg_map.get('s5')
    if s4_reg and s5_r:
        for line in lines:
            m2 = re.search(r'addu\s+\$(\d+),\$' + str(s4_reg) + r',\$' + str(s5_r), line)
            if m2:
                reg_map['s2'] = int(m2.group(1))
                break

    # Check correctness
    target = {'s0': 16, 's1': 17, 's2': 18, 's3': 19, 's4': 20, 's5': 21, 's6': 22, 's7': 23}
    correct = sum(1 for k, v in target.items() if reg_map.get(k) == v)
    checked = len(reg_map)

    detail = " ".join(f"{k}=${v}" + ("✓" if target.get(k)==v else "✗") for k, v in sorted(reg_map.items()))

    ok_frame = '✓' if frame == 88 else '✗'
    ok_blez = '✓' if blez_count == 2 else '✗'
    print(f'{name:20s}: frame={frame}{ok_frame} blez={blez_count}{ok_blez} regs={correct}/{checked} | {detail}')
