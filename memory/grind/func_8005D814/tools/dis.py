#!/usr/bin/env python3
"""Dump our sandbox .o's func_8005D814 as a plain listing (one insn per line,
index-numbered) to tmp/func_8005D814/ours.txt, and the target to target.txt."""
import re, subprocess, sys
F = 'func_8005D814'
o = sys.argv[1] if len(sys.argv) > 1 else 'tmp/sandbox/%s/text1b.o' % F
out = subprocess.run(['mipsel-linux-gnu-objdump', '-d', '-r', '--disassemble=' + F, o],
                     capture_output=True, text=True)
if out.returncode:
    sys.stderr.write(out.stderr); sys.exit(1)
lines = []
for ln in out.stdout.splitlines():
    m = re.match(r'\s+([0-9a-f]+):\s+[0-9a-f]{8}\s+(.*)$', ln)
    if m:
        lines.append(m.group(2).strip())
    elif re.match(r'\s+[0-9a-f]+: R_MIPS', ln) and lines:
        lines[-1] += '   ; ' + ln.split()[-2] + ' ' + ln.split()[-1]
with open('tmp/%s/ours.txt' % F, 'w') as fh:
    for i, l in enumerate(lines):
        fh.write('%3d %s\n' % (i, l))
t = []
for ln in open('asm/funcs/%s.s' % F):
    m = re.match(r'\s+/\* \w+ \w+ \w+ \*/\s+(.*)$', ln)
    if m:
        t.append(m.group(1).strip())
with open('tmp/%s/target.txt' % F, 'w') as fh:
    for i, l in enumerate(t):
        fh.write('%3d %s\n' % (i, l))
print(len(lines), len(t))
