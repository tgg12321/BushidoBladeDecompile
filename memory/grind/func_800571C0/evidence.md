# func_800571C0 — evidence (manual session 2026-09-23)

Closed from cold (no prior ledger work; the only earlier artifact was a
Campaign-4 placeholder body with a wrong signature) in one manual session.

## What the function does
Probes up to four angle steps (0x200..0x800) either side of the owner's facing
(`*(s16 *)(p + 0x1D8)`), at radius `D_800A387C + 800`, using two
`func_80053614` ray tests per step (a vertical ±5 probe, then a line from the
last clear point). Counts clear steps left (`nl`) and right (`nr`), picks the
longer side (ties → `rand() & 1`), writes `nl` waypoints of {s16 x, s16 z, u8 2}
at obj+0x364 (stride 6), copies the first into obj+0x3A0/0x3A2/0x39E, clears
obj+0x398, and returns the chosen count.

## Score trajectory (sandbox --disable all)
287 (stub) → 165 (first transcription) → 138 (`work` is 8 bytes, s16[4]: frame
0xA8) → 55 (sin/cos products as temps before the sums; tail waypoint `=2` store
last; `ret = nl--` + `for (; nl >= 0; nl--)`) → 43 (tail reads D_800A387C
inline, not a local) → 35 (prologue init order; x/z sum temps) → 34 (tail
entry temp `e = obj + nl*6`, gives `addu a0,s0,a0` operand order) → 24 (the
`>> 12` applied at the sum, not in the product temp) → **0**: natural store
order probe.x/y/z then top.x/y/z.

## Ablations at 0
- hitp/workp pointer locals: NOT needed (dropped; the `lw a2,0x78(sp)` spill is
  GCC's own).
- `>>12` placement: must be at the sum for dz; symmetric (both at sum) also 0 — kept symmetric.
- `ang` pre-init before the loop: not needed (normal `for` init).
- tail `ang += 0x200` in the for-increment: 0 (kept there, not mid-body).
- tail `e` temp: needed (inline `obj + nl*6 + K` scores 1 — operand order).
