# func_80061ACC — WIP (register-rotation plateau; same family as func_80061658)

## TL;DR (2026-06-14)
Text-flush sibling of func_800618B4; body differs but the cheat + gap are the
SAME family plateau. HEAD carries `register s32 t asm("$2")` +
`register s32 mask asm("$3")` pins (0 regfix rules). Unpinned masked floor =
**9**, and the diff is ONLY the tail v0/v1 register-rotation:
loads land in v1 / mask(0xFF8080) in v0 (unpinned) vs target loads v0 / mask v1
with `lui v1,0xff` interleaved between loads. Identical rotation gap to
func_80061658/710. Block as part of the family.

## Confirmed
objdiff: only the 9 tail insns differ (lw/lui/ori/sw v0<->v1). Function body
(D_800F1164 two-stage dispatch) matches the target. See
`memory/wip/func_80061658/notes.md` for the ruled-out pure-C forms (4 forms,
none beat floor 9) — not re-derived here.

## Why blocked, not matched
COMPLETED-C needs 0 pins; removing them gives masked floor 9 and the family's
pure-C reorderings don't reproduce the rotation. Family-level permuter or
canonical/plateau decision needed (func_800618B4 already parked).
