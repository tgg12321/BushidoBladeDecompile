# func_800290B8 hypotheses

## Open

- (none; s1 2026-09-28 reached 0 with the Ruling 11 reuse — see evidence.md / ruling11.md)

## Rejected

- One variable per value for the corner row/column vs the marker/triangle index (r11/pv.c):
  21. The marker index spills (8 refs over 2 calls fails the caller-save test) and the column
  lands in v1 (block-local, local-alloc) — ruling11.md (D).
- Corner index written inline without `vtx` (r11/no_vtx.c): 89.
- Markers read as `list[mark].field` instead of a walking pointer (r11/r_index.c): 40.
- SOTN self-assign / dead-store probes on the per-value body (r11/f_*.c): 21, no effect.
- Sanctioned families on the per-value body (fam/*.c, s2): best 4 (init do-while(0) wrap seats
  the marker index in t0; row/column stay v0/v1). Permuter from the per-value body (two runs,
  78k iterations): best 20/1070 (permuter weights), finds stage loop-1 values through `mark`.
