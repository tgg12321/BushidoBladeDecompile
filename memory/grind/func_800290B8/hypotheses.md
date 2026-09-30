# func_800290B8 — hypotheses (ruled out / open)

> Merge provenance (2026-09-30): func_800290B8 was landed COMPLETED-C independently by two
> lanes on 2026-09-28: local main `2b8872877` (manual; ledger: r11/proof.md) and origin/main
> `02e9cfe6b` (manual, cloud container; ledger: ruling11.md, dumps/, fam/, r11/). Both bodies
> byte-match with the same Ruling 11 two-variable reuse. The merged tree keeps the LOCAL body
> in src/code6cac_b_tu2.c (func_80029454 was landed after it in that TU context); origin's
> body is banked as candidate.origin-02e9cfe6b.c. Both lanes' records follow in full.

# [local lane — 2b8872877, landed 2026-09-28]

# func_800290B8 — hypotheses (ruled out / open)

Measured with `sandbox --disable all` on the current build compiler, 2026-09-28.

## Ruled out — spellings without the reused locals (all ≥ 21)
- one-variable-per-value (loop-body `row`/`col`): 21 (rejected/one-var-per-value-21.c)
- function-scope `row`/`col`: 21; block-scoped initialised: 21
- `(idx << 2) + (row << 1) + col`: 21; `(idx * 2 + row) * 2 + col`: 52
- inline `idx * 4 + (i / 2) * 2 + (i & 1)`: 22; inline nested: 52
- types: `u32` row/col 21, `s16` row/col 22, `u32` list index 21, `s16` list index 21
- only the row split (column still reused): 22; only the column split: 2 (register-only)
- permuter from the one-var body: see r11/proof.md

## Ruled out — layout
- hit block after the outer loop (goto from both sites): 50-52 (rejected/out-of-loop-hit-block-52.c)
- `i % 2` for `i & 1`: 34 (signed remainder expansion)
- `max.y < rec->y` operand order (both loops): +2 hunks each

## Open
Nothing at the byte level (0/231). Admission rests on the Ruling 11 layer-2 review.

# [origin lane — 02e9cfe6b, landed 2026-09-28]

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
