#!/usr/bin/env python3
"""Fix lwl/lwr/swl/swr endianness: XOR offset with 3 (big-endian -> little-endian).
Only applies outside #APP blocks (inline asm already has correct offsets)."""
import re
import sys

in_app = False
for line in sys.stdin:
    stripped = line.strip()
    if stripped == "#APP":
        in_app = True
    elif stripped == "#NO_APP":
        in_app = False

    if not in_app:
        m = re.match(r'(\s*)(lwl|lwr|swl|swr)\s+(\$\w+)\s*,\s*(-?\d+)\((\$\w+)\)(.*)', line)
        if m:
            prefix, op, reg, offset, base, rest = m.groups()
            new_offset = int(offset) ^ 3
            line = "{}{}\t{},{}({}){}\n".format(prefix, op, reg, new_offset, base, rest)

    sys.stdout.write(line)
