# motion_SetMotion (code6cac_c_mid.c) — WIP, coupled plateau, needs-decision

## TL;DR (2026-06-16)
402-insn motion-state dispatcher. HEAD byte-matches ONLY via TWO forbidden
cheats that are COUPLED. Exhaustive analysis (session 2) confirms BOTH issues
are genuine plateaus with no pure-C lever — emitting needs-decision.

1. **Register pins** `register s32 result asm("s3")`, `register s32 sel2 asm("s2")`.
2. **regfix subst** `motion_SetMotion: subst "addiu\t$16,$zero,12" "...,13" @149`.

EMPIRICALLY CONFIRMED (session 2, 2026-06-16):
- Pins removed + regfix kept → SHA1 46e36200 (RA mismatch). RA alone blocks match.
- sel=0xD case9/11 + regfix removed → SHA1 82e50f76 (cross_jump merge, from WIP session 1).

Do NOT trust masked sandbox score (stays at 10 regardless — RA swap is invisible).

## Issue 1 — Register Allocation (PLATEAU, proven)
Target: result→$s3, sel2→$s2. Pin-free gives result→$s2, sel2→$s3 (swap, confirmed).

GCC 2.7.2 priority formula (from tools/gcc-2.7.2/global.c:604):
  `floor_log2(n_refs) × n_refs / live_length × 10000 × size`

Measured values:
- result: n_refs=6 (init=0, 3×result=1, `if (result!=0)`, `return result`),
  live_length≈400 (d74→1384, all blocks), priority = floor_log2(6)×6/400×10k = 2×6/400×10k = 300
- sel2: n_refs=3 (init=-1, load at load_sel2, use at sel_dispatch move a1,s2),
  live_length≈124 (d7c→fe0 skipping early-exit body dc4-e30), priority = 1×3/124×10k = 242

result (300) > sel2 (242) → result gets $s2. To flip: sel2 needs n_refs≥4.
Any 4th ref to sel2 CHANGES THE BINARY — oracle at dc8-dcc shows `li a1,-1` NOT
`move a1,s2`. No pure-C way to add an invisible ref.

The "target uses higher reg" case (result in $s3 > $s2) is the RARER case per
register-alloc-pure-c rule ("pin may be closer to justified").

## Issue 2 — Cross-Jump Merge (PLATEAU, proven)
Target: case 9/11 at fc0 = `j fd4; li s0, 13`; D_800A3207==3 non-10 at ea0 = `j fd4; li s0, 13`.
Both blocks separate in oracle.

Mechanism: GCC's `find_cross_jump` (jump.c, minimum=2) merges blocks with identical
2-insn suffix. With case 9/11 sel=0xD: both blocks → `li s0,13; j sel_dispatch` → merge fires
→ SHA1 82e50f76. With sel=0xC: suffix differs (12≠13) → no merge → regfix patches 12→13.
No pure-C structural change can produce two SEPARATE `li s0,13; j sel_dispatch` blocks without
one of them having sel=0xC (then needing regfix) or the cross_jump making them one block.

Mixed-exit-forms lever (cross-jump-store-tail-merge rule) requires one path to be an inline
return. Neither case 9/11 nor D_800A3207==3 non-10 can `return` mid-function — both must
go to sel_dispatch to call func_8006BEC4. Lever not applicable.

## Ruled out (do not re-derive)
- `case 9/11: sel=0xD` + rule removed: SHA1 82e50f76 (cross_jump merge)
- Pins removed + regfix kept: SHA1 46e36200 (RA mismatch, CONFIRMED this session)
- Declaration-order swap: masked noise, RA swap persisted (session 1)
- `func_8006BEC4(0xA, sel2)` at early exit: changes binary (li a1,-1 → move a1,s2)
- Any add of refs to sel2 to raise priority: all change binary

## Next steps for Adjudicator
- (a) Point to any pure-C lever that flips RA (sel2 priority > result) without changing binary
- (b) Point to any structural approach preventing cross_jump merge with both blocks emitting 13
- (c) Or confirm as documented plateau → park
