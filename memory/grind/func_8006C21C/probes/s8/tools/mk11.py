"""cand11 = cand10 + i renamed to `work` + admission annotations (records live in admission.md)."""
import re
NL = chr(10)
t = open('tmp/c21c/cand10.c').read()
L = t.split(NL)
# rename i -> work inside the function body only
start = next(k for k, l in enumerate(L) if l.startswith('void func_8006C21C('))
for k in range(start, len(L)):
    L[k] = re.sub(r'\bi\b', 'work', L[k])
t = NL.join(L)

t = t.replace('    s32 mode;' + NL,
'''    /* FAKE: constant-holder (named-local-fake-exception) -- the 0 passed as
       func_8006E480's second argument at all three call sites. Set once and
       live across every call, so global.c gives the once-set constant pseudo
       callee-save $s5 (target: `addu $s5,$zero,$zero`, then `addu $a1,$s5,$zero`
       at the phase-3 and phase-5 calls; cse folds the phase-1 read to 0 inside
       the entry block). Literal 0 at every call measured worse; receipts in
       memory/grind/func_8006C21C/admission.md "mode". Same shape as the
       siblings func_800753D8 (`zero`) and func_8007636C (`mode`). */
    s32 mode;
''', 1)
t = t.replace('    s32 work;' + NL,
'''    /* Holds several values (Ruling 11, ordinary-c-judge-decidable.md): the
       phase-2 unlock-bit index, the phase-4 sprite index, the phase-6 tile
       row, and the phase-8 gauge level read per player. One variable is what
       the target's allocation requires (global.c: the merged pseudo outranks
       `j` for $fp); proof in memory/grind/func_8006C21C/admission.md "work". */
    s32 work;
''', 1)
t = t.replace('    u8 *cells;' + NL,
'''    /* the sprite sheet's 8-byte cell array, which starts just past the sheet's
       12-byte header (SprtHdrA / SprtEntA, read by func_8007352C). Ruling 9:
       one meaning, header + 0xC at every write; memory/grind/func_8006C21C/
       admission.md "cells". */
    u8 *cells;
''', 1)
assert t.count('admission.md') == 3
t = t.replace(' * s8 (2026-09-28): bar 1\'s vertices use PsyQ setXYWH (byte-neutral, 41/622).',
              ' * s8 (2026-09-28): bar 1\'s vertices use PsyQ setXYWH (byte-neutral, 41/622);\n'
              ' * mode set once and used at all three calls; i renamed `work` (Ruling 11 E).', 1)
open('tmp/c21c/cand11.c', 'w', newline=NL).write(t)
print('ok')
