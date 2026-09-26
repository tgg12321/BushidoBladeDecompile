# func_8002DE20 — tree-wide before/after record for the Q11 gtemacro change (2026-09-26)

Requirement: inline-asm-policy.md § Scorer ruling (owner, 2026-09-25) item 3 ("The change records
sandbox distances before and after for every function. Any function whose distance changes must
carry a qualifying unit"), applied to the Q11 recognizer update (layer-2 objection 1).

Method: tools/treewide.py (copy of tmp/func_8002DE20/treewide.py), the same method as 15597a423.
For EVERY src/*.c (36 files; every function on main) and EVERY banked .c under memory/grind/
(1117 files: candidates, rejected bodies, variants), it computes the sandbox's stripped input,
`inlineasm.strip_cheat_asm_file(text, keep_gte_macro_units=True)`, with the HEAD
engine/gtemacro.py (before) and with the Q11 one (after). A function's sandbox distance is a
function of its stripped TU (same pipeline, same target), so an unchanged stripped input
means an unchanged distance. The default strip (keep_gte_macro_units=False, the completion
gate's view) is compared too. Raw data: r11/treewide.json.

Result (1153 files scanned, 0 errors):
- all 36 src/*.c: stripped input IDENTICAL, so no function on main moves;
- default strip: identical in all 1153 files;
- keep-mode stripped input changed in 11 files, all func_8002DE20 bodies. Each carries the 30
  statements the Q11 pins complete: gte_ldv0 / gte_rtv0 / gte_stlvnl / gte_ApplyRotMatrix,
  stripped 12 -> 0.

Distances of the 11 changed bodies. "Before" = HEAD engine, default strip. "After" = Q11
engine. Q11 strips nothing from these bodies, so the "after" figure equals the
--keep-cheat-asm score. Both were measured on the 0($12) spelling, which is byte-identical
(maspsx proof); for candidate.c the landing sandbox confirmed 0 on the staged tree.
| body | before | after |
|---|---|---|
| candidate.c | 10 (497 insns) | **0** (506) |
| r11/s_helper_shared.c | 10 | 0 |
| r11/final_pv.c | 100 | 90 |
| r11/s_inline.c | 100 | 90 |
| r11/s_helper_pv.c | 100 | 90 |
| r11/ext_self.c / ext_chain.c / ext_use.c / ext_usevar.c | 100 | 90 |
| r11/ext_dead.c | 112 | 102 |
| r11/ext_alias.c | 188 | 179 |
Every changed body carries qualifying units (item 3's condition); no other function changes.
