# Hypothesis ledger — func_8003B5A4

## s1 (2026-08-07, recon)

- **H1 — pointer-local + store-before-load order in cases 3 and 1** — CONFIRMED, closed
  the function. Statement: the 19-insn floor (two reg-rotation/reorder clusters) comes
  from caching bytes through an `s32 data` local instead of the target's shape: fresh
  `u8 *p` local, store `p+1` to D_800A3844 BEFORE loading through p, load each byte at
  its use site. Mechanism: the pointer pseudo dies at its final load, so GCC reuses its
  register for the loaded byte (divmod-coalesce-style reuse); removing the early-cached
  byte pseudos kills the extra live ranges that rotated RA ($3<->$4) and dragged lbu's
  early (the reorder rules). Probe: rewrote both cases, sandbox --disable all. Result:
  19 -> 0. Verdict: CONFIRMED.

Frontier: empty — candidate at distance 0, awaiting integration (rule retirement +
oracle verify + layer-2 review by the driver/operator).
