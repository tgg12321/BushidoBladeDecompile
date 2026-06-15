# func_80061EC0 — WIP (register-rotation plateau; same family as func_80061658)

## TL;DR (2026-06-14)
Nested-bool sibling (byte offset 2/3) of func_800618B4. Same cheat + gap as the
family. HEAD carries `register s32 t asm("$2")` + `register s32 mask asm("$3")`
pins (0 regfix rules). Unpinned masked floor = **9**; objdiff shows ONLY the
tail v0/v1 register-rotation differs: loads land in v1 / mask(0xFF00FF) in v0
(unpinned) vs target loads v0 / mask v1 with `lui v1,0xff` interleaved between
loads. Body (D_800F1160 offset-2/3 dispatch) matches the target. Block as part
of the family; ruled-out pure-C forms live in
`memory/wip/func_80061658/notes.md` (not re-derived here).

## Why blocked, not matched
COMPLETED-C needs 0 pins; removing them gives masked floor 9 and the family's
pure-C reorderings don't reproduce the rotation. Family permuter/canonical
decision needed (func_800618B4 already parked).
