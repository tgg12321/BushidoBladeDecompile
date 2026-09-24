# func_8002F2D0 — hypotheses / ruled out (manual session 2026-09-22)

Scores are sandbox --disable all (honest). Variants live in tmp/f2d0/.

## Ruled out
- declaration-order permutations (12 random): all identical (24) — pseudo
  numbering is not the lever here.
- scr assignment placement (top of function / before ang_z): no change; the
  const load is re-sunk.
- mat as MATRIX* / identity written through mat (Q1/Q2): CSE canonicalizes the
  stores back to scr+off — no change (19).
- `mat` var removed, `(s32 *)(scr + 0xD8)` inline at each use (k1): identical.
- det spellings: single expression (72, loses the early m00 mult), separate d0
  folded into the expression without being placed after c0 (72), d1/det in
  other operand orders (80-85). Winner: F1 (d0 placed right after c0).
- sum spellings: `c1*c1 + c0*c0` (26), `sum = a; sum += b` (26), sum reusing
  c2 (86), reusing det (72 at the time), inline-function helper for the sqrt
  (27), copy `x = sum` inside else (24), macro-style duplicate evaluation of
  the sum expression in the arms W1-W3 (19 — CSE removes the duplicates),
  reusing parameter a0 for the sum (no change).
- `x = sum; if (sum...) lut[x]` (G1a/G1b, 20): x lands in a0 but CSE rewrites
  x back to sum in the LUT arm and the first else sub-block.
- i2 reused for the last ratan2 result (R1, 21): overshoots — i2 then outranks
  the a1 param (s3/s4 swap).
- c2 reused as i2 (U1, 92).
- full reuse c0->i0->ang_y, c1->i1->mat (V1/V2, 47): sum fixed but scr
  (12363) outranks both reused vars and takes s0.

## Sum -> $a0: SOLVED 2026-09-22 (see evidence.md FINAL)
Closed by giving the sum a lower-numbered preference ($a0) rather than removing
{s0,s1}: reuse `sum` for the LUT byte so the `<< 16` reads it. Also ruled out on
the way: m20's element variable reused as the sum (S1/S2, 69 — sum escapes s0
but block-0 locals reshuffle); inline helper with an unmodified parameter
(27); full-reuse V1 (47). Two permuter campaigns (F1 base 100 -> 55 via a
mat copy; M1 base 30, 39k iterations, flat) found nothing for the sum.
Final devices in place: two single-level do-while(0) wraps (i2 div,
gte_SetRotMatrix island) — the island wrap replaced the permuter's
`new_var = scr + 0xD8; mat = new_var;` copy (same effect, established device).
