"""Relocate the records of the functions the text1b | text1b_tu1c boundary move (move.py) takes
from text1b_tu1c.c to text1b.c: the queue item's "file" field and the grind state.json "file"
field, nothing else (the rodata-align doc section 7 procedure). Run from the repo root, only
while holding the landing lock. Textual edits of the one field, so the rest of each file keeps
its bytes."""
import re
from pathlib import Path

QUEUE = ['func_8005C8A8', 'func_8005D554']
STATE = ['func_8005C8A8', 'func_8005D554', 'func_8005D814', 'func_8005E54C', 'func_8005F1C8']
OLD, NEW = '"file": "text1b_tu1c"', '"file": "text1b"'

p = Path('engine/queue.json')
q = p.read_bytes().decode('utf-8')
for f in QUEUE:
    pat = '"func": "%s",\n      %s,' % (f, OLD)
    assert q.count(pat) == 1, f
    q = q.replace(pat, '"func": "%s",\n      %s,' % (f, NEW))
p.write_bytes(q.encode('utf-8'))
for f in STATE:
    sp = Path('memory/grind/%s/state.json' % f)
    s = sp.read_bytes().decode('utf-8')
    assert s.count(OLD) == 1 and re.search(r'"func": "%s"' % f, s), f
    sp.write_bytes(s.replace(OLD, NEW).encode('utf-8'))
print('queue: %d items, state.json: %d files' % (len(QUEUE), len(STATE)))
