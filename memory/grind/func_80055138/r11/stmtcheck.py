"""Ruling 11 (C)(2) receipt: the reuse body and a split twin have the SAME statement list.
usage: python3 tmp/func_80055138/r11/stmtcheck.py <reuse.c> <twin.c> <carrier> <name>...
Strips comments, deletes the declaration lines of <carrier> and of every <name>, renames
every <name> back to <carrier>, whitespace-normalizes, and compares."""
import re, sys

reuse, twin, carrier, names = sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4:]


def norm(path):
    s = open(path).read()
    s = re.sub(r'/\*.*?\*/', '', s, flags=re.S)
    for n in [carrier] + names:
        s = re.sub(r'^\s*(?:s32|u8|u32) %s;\s*$' % n, '', s, flags=re.M)
    for n in names:
        s = re.sub(r'\b%s\b' % n, carrier, s)
    return ' '.join(s.split())


a, b = norm(reuse), norm(twin)
print('IDENTICAL statement lists' if a == b else 'DIFFERENT')
if a != b:
    import difflib
    for l in difflib.unified_diff(a.split(';'), b.split(';'), lineterm='', n=0):
        print(l)
