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
