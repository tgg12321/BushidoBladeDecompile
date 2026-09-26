# func_80077D94 — evidence (manual session 2026-09-25)

Floor trajectory (sandbox --disable all --candidate): 118 (first structural decode)
→ 74 → 57 (insn count exact) → 48 → 41 → 32 → 7 → 3 → 2 → 0 once the
hand-transcribed jtbl_80015A54 array was removed from src (the last 2 were the
`lw v0,%lo(jtbl)(at)` addend artifact of both tables co-existing in the sandbox build).
Oracle green with the body spliced + jtbl array deleted.

## Levers that each closed a measured hunk

- Prologue fade: compute `v` first, then `has_color = 1`, then `r = g = b = v`
  (b stored first) — otherwise in[] is reloaded after the has_color store.
- Header tables: walking `hp` (single variable, seats $s1; arg0 → $s2). Indexed
  `hp[i]` in the case loops gives the right schedule but swaps $s1/$s2 (+40).
- For-increment order: `hp++, i++` (cases 0/4); `x += 0x80, hp++, i++` (cases 2/1).
  Found by a 192-variant permutation sweep of statement order × increment order
  (4! statement orders × 8 increment placements).
- Final loop: `hp[i]` indexed (biv eliminated, s0 = 4i); window address
  `i * 4 + (s32)win` (vs `&win2C[i]`: 1/468, addu operand order only).

## FAKE-annotated constructs — ablations on the final body (all vs 0/468)

Measured 2026-09-25 by substituting ONE construct into the final body
(tmp/f77/b_*.c; scratch).

| construct | alternative spelling | score |
|---|---|---|
| `abr = 0x20` holder ($s7) | inline literal `0x20` at all 5 calls | 12/465 |
| | literal only in the final loop | 1/468 |
| | `s16 abr` | 10/473 |
| | `u8 abr` | 5/468 |
| | `abr = 0x20;` just before the switch | 4/467 |
| `j = i * 4 + 0x38` (case 3) | `((Ctx77D94 *)D_800A35F8)->img38[i]` | 9/467 |
| | `*(s32 *)(D_800A35F8 + i * 4 + 0x38)` | 9/467 |
| | earlier base: `(i*4+0x38)` parenthesized / `(D+0x38)[i]` / `D + 0x38 + i*4` | 41 each (c3d/c3e/c3f) |
| | earlier base: `j = i + 14; ((s32 *)D_800A35F8)[j]` | 33 vs 32 for `j = i*4+0x38` on that base (addu operand order) |
| `t = D_800A35F0 - 60` (fade-out) | `w->off - (D_800A35F0 - 60)` inline | 16/469 |
| | `60 - (D_800A35F0 - w->off)` | 16/469 |
| final-loop per-arm r/g/b chains | one shared chain after the if/else | 26/467 |
| prologue per-arm has_color/r/g/b | arms set v only; else skips via goto | 23/470 |

Mechanisms: $s7 constant seat across calls (global.c, callee-saved for a
call-crossing pseudo); loop.c giv reduction of `0x38 + 4i` (fold otherwise moves
0x38 into the load displacement); fold reassociation of `off - (cnt - 60)`;
cross-jump of identical arm tails (a single join store gets its own pseudo and a
`move`). Reviewer (layer-2, 2026-09-25) independently reproduced literal 12/465
and img38 field 9/467.
