"""Print every target instruction of func_80055138 that pairs one of the given registers with
another register as (dest, first source operand) — the pairs global.c set_preference can turn
into a hard-register preference. Loads/stores (memory source) and branches are skipped.
usage: python3 tmp/func_80055138/r11/pairs.py v1 a0 a1"""
import re, sys

want = set(sys.argv[1:]) or {'v1', 'a0', 'a1'}
for l in open('asm/funcs/func_80055138.s'):
    m = re.search(r'/\* \S+ (8005[0-9A-F]{4}) \S+ \*/\s+(\S+)\s+(.*)', l)
    if not m:
        continue
    addr, op, args = m.groups()
    if op in ('j', 'jal', 'jr', 'beq', 'bne', 'beqz', 'bnez', 'nop', 'sw', 'sh', 'sb', 'break',
              'mult', 'multu', 'div', 'mfhi', 'mflo', 'lui') or op.startswith(('lb', 'lh', 'lw')):
        continue
    regs = re.findall(r'\$(\w+)', args)
    if len(regs) < 2:
        continue
    dest, first = regs[0], regs[1]
    for r in want:
        other = first if dest == r else dest if first == r else None
        if other and other not in (r, 'zero', 'at'):
            print(addr, op, args.strip(), '->', r, 'with', other)
