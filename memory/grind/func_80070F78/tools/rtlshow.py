"""rtlshow.py dumpfile uid_lo uid_hi : print compact RTL insns (uid order as in file)"""
import sys, re
t = open(sys.argv[1]).read()
lo, hi = int(sys.argv[2]), int(sys.argv[3])
# split into top-level insns
parts = re.split(r'\n(?=\((?:insn|call_insn|jump_insn|code_label|note|barrier) )', t)
for p in parts:
    m = re.match(r'\((insn|call_insn|jump_insn|code_label|note|barrier) (\d+)', p)
    if not m: continue
    uid = int(m.group(2))
    if lo <= uid <= hi:
        s = re.sub(r'\s+', ' ', p)
        s = re.sub(r'\(expr_list:REG_\w+.*$', '', s)
        s = re.sub(r'\(insn_list.*$', '', s)
        print(s[:230])
