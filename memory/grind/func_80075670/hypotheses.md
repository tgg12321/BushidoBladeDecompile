# Hypothesis ledger — func_80075670

## H1 — single-variable base merges pseudos and tanks allocation priority (session 1) — CONFIRMED, function closed
- Statement: the 20-diff register rotation is caused by one `base` local
  spanning the whole function; the original used separate top-half and
  bottom-half pointer locals plus a block-local pointer in the post-first-loop
  store, letting global.c allocate the bottom base FIRST ($a1).
- Mechanism: GCC 2.7.2 global.c allocno ordering ~ refs/live-length; a
  unioned 0-101 live range drops the merged pseudo below the two hoisted
  loop constants, inverting the $a1/$a2/$a3 assignment; a split bottom-half
  var (short range, loop-weighted refs) sorts first.
- Probe: split into base/work/q (session-1 probes 1+2).
- Result: 20 → 4 → **0**. CONFIRMED.

## H2 — post-loop read+write must share one reload through the same pointer (session 1) — CONFIRMED
- Statement: target's single `lw gp` + `addu $a3,$a3,$v1` self-update means
  the original read `[q + arg1 + 0x68]` and wrote `[q + ((arg1+1)&1) + 0x68]`
  through the SAME fresh pointer local in one statement.
- Mechanism: CSE shares q's load across both address computations; q dies at
  the write-address add so local-alloc coalesces the sum onto q's register.
- Probe: single-statement form (probe 2). Result: closed the last 4 diffs. CONFIRMED.
