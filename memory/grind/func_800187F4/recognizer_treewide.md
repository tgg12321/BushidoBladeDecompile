# func_800187F4 — tree-wide before/after record for the Q29 gtemacro change (2026-09-28)

Requirement: inline-asm-policy.md § Scorer ruling (owner, 2026-09-25) item 3 ("The change records
sandbox distances before and after for every function. Any function whose distance changes must
carry a qualifying unit"), applied to the recognizer update of § Per-function grant:
func_800187F4 item 2 (owner ruling 2026-09-28, Q29; record commit 206e77db8).

The change (engine/gtemacro.py): PINNED gains the seven inline_o.h 4.3 macros this body uses
that were not yet pinned (gte_ldlvl :104-109, gte_lddp :144-147, gte_rtv0tr :451-455, gte_sqr0
:646-650, gte_gpf0 :721-725, gte_gpl12 :726-730, gte_stlvl :898-903), each cut byte-exact from the
pinned copy by tools/pin_gen.py (asserts the header sha256 76f28032...); DMPSX_WORDS gains the four
Q29 pairs (0x0000027f->0x4A480012, 0x00000f3f->0x4AA00428, 0x000012ff->0x4B90003D,
0x0000133f->0x4BA8003E).

Method: tools/treewide.py (the func_8002DE20 method, memory/grind/func_8002DE20/tools/treewide.py,
paths changed only). For EVERY src/*.c and EVERY banked .c under memory/grind/, it computes the
sandbox's stripped input, `inlineasm.strip_cheat_asm_file(text, keep_gte_macro_units=True)`, with
the HEAD engine/gtemacro.py (before; copy tmp/f187/gtemacro_head.py) and the changed one (after),
and compares the default strip (the completion gate's view) too. A function's sandbox distance
is a function of its stripped TU, so an unchanged stripped input means an unchanged distance.
Raw data: recognizer_treewide.json.

Result (1453 files scanned, 40 of them src/*.c, 0 errors):
- all 40 src/*.c: stripped input IDENTICAL, so no function on main moves;
- default strip: identical in all 1453 files;
- keep-mode stripped input changed in exactly 2 files, both func_800187F4 bodies:
  candidate.c (sha256 42b045e9...) and candidate_copyfree.c (9b6a8e18...): stripped 29 -> 0
  statements; recognized unit statements 12 -> 85 (the two gte_Lzc units were already pinned; the
  change completes gte_ldv0+gte_rtv0tr+gte_stlvnl runs and every ldlvl / sqr0 / gpf0 / gpl12 /
  lddp / stlvl unit).

Distances of the 2 changed bodies (`sandbox --disable all --candidate`):
| body | before (HEAD engine) | after |
|---|---|---|
| candidate.c | 147 (570 build insns; evidence.md [s2 close]) | **0** (644/644) |
| candidate_copyfree.c | 147 (570) | **2** (644/644) |
The "after" figures equal the real-pipeline measurement (tools/fast.sh: 0 and 2 differing lines),
so the sandbox now scores these bodies as written. Both carry qualifying units (item 3's
condition); no other function changes.
