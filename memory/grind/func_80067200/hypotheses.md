# func_80067200 — hypotheses / measured spellings (2026-09-24)

| spelling | score | note |
|---|---|---|
| first draft: s32 base/mask, ang after scale, vx-first | 34 | mask shares s3 with divisor; no preheader move |
| + `dv = mask` copy before loop (s32) | 13 | move coalesced away (same hard reg) |
| + dv copy, vy-first angles | 7 | amask conflicts with dv -> amask s5, mask s2 (RA model reproduces 20/20) |
| dv copy at 5 other positions | 7..30 | before-test copy always overlaps amask |
| dv copy inside loop (top / before &=) | 28 | cse copy-propagates dv away (mask canonical) |
| angle block before scale block (s32 + dv) | 7 | neutral on s32 chassis |
| s16 mask, s32 base, angle-first | 11 | |
| s16 mask, s32 base, angle-after | 7 | |
| s16 mask + s16 base, angle-after | 19/25 | |
| **s16 mask + s16 base, angle-first, vy-first** | **0** | candidate.c — no copy variable, no FAKE constructs |
| same, vx-first | 6 | r[0]/r[1] stack store order |

Killed: the `dv` copy-variable family entirely (not needed; the move is the
combine-simplified extend of an s16 mask).
