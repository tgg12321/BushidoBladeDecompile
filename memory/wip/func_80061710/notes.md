# func_80061710 — WIP (register-rotation plateau; identical sibling of func_80061658)

## TL;DR (2026-06-14)
Byte-for-byte the SAME plateau as `func_80061658` (only the mask constant
differs: 0x10FF10 vs 0x10FFFF). HEAD carries `register s32 t asm("$2")` +
`register s32 mask asm("$3")` pins, 0 regfix rules. Unpinned honest masked
floor = **9**, the identical v0/v1 register-rotation + interleaved-mask
schedule gap. Block as part of the family — see
`memory/wip/func_80061658/notes.md` for the full ruled-out form list (4 forms,
none beat 9) so it is NOT re-derived here.

## Confirmed identical gap
diff vs target: loads land in v1 / mask in v0 (unpinned) vs target loads v0 /
mask v1 with `lui v1,0x10` interleaved between loads. Masking can't normalize
the interleaving → masked diff 9. Same as func_80061658.

## Why blocked, not matched
COMPLETED-C needs 0 pins; removing them gives floor 9 and the family's pure-C
forms don't reproduce the rotation. Same decision as func_80061658 /
func_800618B4 (parked register-rotation pin cluster).
