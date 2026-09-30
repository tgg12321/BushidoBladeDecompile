# func_8002EBDC cluster — tree-wide before/after record for the gte_ldlv0 / gte_SetRotMatrix PINNED change (2026-09-30)

Requirement: inline-asm-policy.md § Scorer ruling (owner, 2026-09-25) item 3, and the 2026-09-26
class grant's (A) ("Adding a macro's lines to PINNED is an engine: commit, reviewed by a layer-2
cheat-reviewer who checks the excerpt against the pinned header").

The change (engine/gtemacro.py): PINNED (inline_o.h 4.3) gains gte_ldlv0 :95-103 (sha256
25eddbcf...) and gte_SetRotMatrix :272-284 (1b3eac43...), cut byte-exact by tools/pin_gen.py
from the pinned copy (silent-hill-decomp@a1f407cb include/psyq/inline_o.h, file sha256
76f28032e381a78a4c96347eeee753150cfb55b9f0f0be414fd5040bf4c6e47d, re-downloaded 2026-09-30;
the xenogears-decomp@54d7ef3e copy downloaded the same day has the same sha256). No DMPSX word,
no recognizer logic, no admission path changes.

Method: tools/treewide.py (the func_800187F4 method; "before" read from git show HEAD:engine/gtemacro.py at run time): for EVERY src/*.c and
banked .c under memory/grind/, the sandbox's stripped input (keep_gte_macro_units=True) and the
default strip, HEAD engine/gtemacro.py vs the changed one. Raw: recognizer_treewide.json.

Result (2187 files, 53 src/*.c, 0 errors; re-run after the review nits: 2192 files, same 10 changes, 0 errors):
- all 53 src/*.c: stripped input IDENTICAL (no function on main moves);
- default strip: identical in all 2187 files;
- keep-mode stripped input changed in 10 files, all bodies of func_8002EBDC / func_8002F2D0 /
  func_8002F770 (candidates, their rejected variants, and the two inline_c.h-rtv0 alternatives,
  which gain only the SetRotMatrix unit).
Real sandbox after the change (`sandbox --disable all --candidate`): func_8002EBDC 0/182,
func_8002F2D0 0/270, func_8002F770 0/298 (0 source-level, 0 operand-only hunks each), matching the
earlier no-strip measurement; func_8002D780 unchanged 0/202.
engine test: 1065 passed, 2 failed — the 2 failures (departures real-history clone) are
PRE-EXISTING: HEAD's engine alone gives 1043 passed, the same 2 failed; +22 new checks all pass.
