"""Textual-identity check of the TU split: the function text moved into code6cac_b3_post.c must be
byte-for-byte the tail of the in-place (pre-split, already-converted) code6cac_b.c, and the split
code6cac_b.c must be the in-place head minus only the moved externs."""
import sys
inplace = open(sys.argv[1], encoding='utf-8').read()
head = open(sys.argv[2], encoding='utf-8').read()
post = open(sys.argv[3], encoding='utf-8').read()
k = inplace.index('/* kengo:LOW  |  su_menu_vs/_DispSamnailWindow')
ip_head, ip_tail = inplace[:k], inplace[k:]
lines = ip_tail.split('\n')
assert lines[1] == 'INCLUDE_ASM("asm/funcs", func_80034708);'
moved_text = '\n'.join(lines[2:]).lstrip('\n')
assert post.endswith(moved_text), 'moved text differs'
print('moved block identical:', len(moved_text), 'bytes,', moved_text.count('\n'), 'lines')
import difflib
d = [l for l in difflib.unified_diff(ip_head.rstrip('\n').split('\n'), head.rstrip('\n').split('\n'), lineterm='', n=0)
     if l.startswith(('+', '-')) and not l.startswith(('+++', '---'))]
print('head lines removed/added:', d)
