# Hypothesis ledger — func_800720FC

## Open (s2, 2026-09-29)
- `cells` (function-scope, 6 writes: `s.header + 0x18` once, `s.header + 0xC` five times) is required for the
  target's s2 seat at every site; admission ruling not yet settled. Ruling 9: BASE `s.header` (member of a local
  object), K 0x18/0xC, one consumer `s.cells = cells;` — but prong (c) (each write read once) fails for the first
  block (read by the loop). Try: a separate first-block local + Ruling 9 for the rest; else Ruling 11 with dumps.
- D_800A35C8/CA hoist under -G0 (see evidence s2).

## Ruled out (s2)
- `D_8009BCB4[mode + 4]` (no distribution; 103+), `(D_8009BCB4 + 4)[mode]` (cse relates the scroll base; 103).
- flat grid index `((s32 *)arg1)[i * 2 + j + 3]` / `[j + i * 2 + 3]` / `((s32 *)arg1 + 3)[i * 2 + j]` (136: the
  shared `i*2+j` becomes its own biv); `cellp = (s32 *)arg1 + i*2 + j; cellp[3]` (147).
- confirm `action` as s32 (no andi at all), u16/s16/u8 with explicit masks (see evidence).
- `j += 3` biv form of ctx (126: address giv still reduced to a pointer).

## s2 closing (2026-09-29)
- WITHDRAWN: TU split of text1b.c at func_8006F97C + Q21 per-file D_800A35C8 declarations (layer-2 FAIL on
  evidence; split/tu_boundary.md "Superseded"). Not needed: the timer pointer alias reaches the target with
  the single array declaration (timers/README.md).
- RULED OUT for the timers (all measured, cc1 and cc1psx agree): array [0]/[1] either order, *(p+1),
  sized [2], struct fields either order (4-6, hoisted base). Second scalar handle D_800A35CA reaches 0 but is
  a second C handle for the bytes -> rejected.
- RULED OUT for the page table: every one-base spelling on D_8009BCB4 (14-32); resolved by reading pages
  through D_8009BCC4[page - 4] (one table, no out-of-object reach).
- `cells`: split / partial splits / no-variable all miss (r11/README.md) -> Ruling 11 package.
