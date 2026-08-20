# Hypothesis ledger — func_800324D0

## [s1] 2026-08-20 (recon)

1. **H1 — the 27 is a pure 3-cycle register rotation (walker/cmd/val), no
   shape or schedule residual.** Probe: objdump diff of pin-free sandbox .o vs
   target. Result: confirmed — 27 rename diffs, 68/68 insns, andi + single
   sltiu present. **CONFIRMED.**
2. **H2 — the rotation is a global-RA allocation-order effect, not a tie.**
   Probe: .lreg/.greg + allocno_compare arithmetic. Result: order is
   val(4.95) > cmd(4.2) > byte(3.05) > pad(2.98) > walker(1.55); inversion by
   ref/live tweaks needs ~61 weighted walker refs. **CONFIRMED (and the
   priority-lever class KILLED).**
3. **H3 — a bare pointer-alias copy can plant a walker preference.** Probe:
   `q = v1` alias in the ≥0x80 arm, sandbox + .greg census. Result: flat 27,
   census unchanged (cse coalesces copies pre-RA). **KILLED.**
4. **H4 — walker full-pref $3 via an overlapping block-0 `base`/`ff` pair
   closes the function.** Mechanism: local-alloc ff→$2, base→$3;
   set_preference PLUS-lookthrough on `ptr = base + 5`; prune_preferences →
   regs_someone_prefers {3} on val/cmd; find_reg pass-0 cascade. Probe:
   the base/ff spelling in src, sandbox. Result: **score 0, 68/68, measured
   twice (before and after semantic renames). CONFIRMED — candidate-ready.**

## Frontier
Candidate at sandbox 0 in src/. Remaining risk is review-side only (layer-1 /
Judge assessment of `base`/`ff` as ordinary live C vs a construct needing a
family claim). If bounced on construct grounds, the fallback axis is a
different honest spelling that yields the same walker preference — e.g. other
natural two-consumer intermediates in block 0 — NOT priority tweaks (killed)
and NOT bare aliases (killed).
