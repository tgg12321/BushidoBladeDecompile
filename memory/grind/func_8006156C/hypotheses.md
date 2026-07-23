# Hypothesis ledger — func_8006156C

## s1 (recon, 2026-07-22) — CLOSED, MATCH at distance 0

- H1 (CONFIRMED): The prior sweep only tested ARRAY-INDEX access (`arg0[i]`
  with a reused temp), which lets GCC reschedule the mask-const construction.
  The COMPLETED-C sibling func_800618B4 uses a WALKING POINTER (`*p++`) + mask
  store LAST. Mechanism ([[walking-pointer-serializes-parallel-loads]]): the
  post-increment pointer dependence serializes the 3 loads so GCC keeps
  per-element lw/sw, and the `0xFF8080` mask stored last lets its lui/ori
  schedule into the freed load-delay slots of the 2nd/3rd loads.
  Probe: `p=arg0; D_1140=*p++; D_1144=*p++; D_1148=*p; D_A3464=0xFF8080;`
  Result: sandbox --disable all → score 0 (was 6 with array-index+mask-first).
  cheat-reviewer: PASS (independently re-ran sandbox=0, confirmed zero cheats,
  confirmed 3 COMPLETED-C siblings use the identical idiom).
  → candidate.c updated; forbidden 2-pin form retired.
