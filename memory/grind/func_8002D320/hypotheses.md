# Hypothesis ledger — func_8002D320

## Session 1 (recon, 2026-08-31)

H1 — "The queue distance 32 is stale chassis noise; an honest clean body built
from the retired chassis minus cheats + the brief's levers 1+2 + the twin's
narrow island forms lands near zero."
  mechanism: the 13 retired regfix rules were calibrated against a pinned,
  barrier-laden body whose .word swc2 spellings the sandbox stripped; and the
  flag-shifted argument geometry frees every register seat target uses.
  probe: apply body, `sandbox --disable all`.
  result: score 3 (118/120) — only the tail 0/1 diamond diverged.
  verdict: CONFIRMED.

H2 — "The tail diamond (`bnez; move v0,zero; addiu v0,1` unfolded in target)
cannot be reached by plain-C tail spellings; jump.c's store-flag if-conversion
folds every single-set arm form to slt+xori."
  mechanism: jump.c store-flag conversion (single-set 0/1 arms, condition ==
  value); same symptom class as func_80078EC0 and the twin's L1.
  probe: measured v1 plain early-return (3), v2 result-carrier nest (4/119),
  v3 goto-reject (3, byte-identical), v4 inverted sense (3, byte-identical);
  plus the twin's session-2 six-shape census on the identical diamond (shared
  end label, reversed compare, if/else both arms, ternary, test-order swap,
  double-goto-reject — three byte-identical, three worse).
  verdict: CONFIRMED (pure-C axis dead for the diamond).

H3 — "The twin's L1 staged-z closure (`{ z = 0; return z; }`, two-statement arm
breaks the single-set precondition) transfers to this function."
  mechanism: two-set arm defeats jump.c store-flag fold; z is an existing local
  (rotated Z) provably dead after `z * z`; family staged-value-reused-variable.
  probe: apply + sandbox.
  result: **score 0, 120/120**.
  verdict: CONFIRMED — CLOSING FORM.

## Live frontier

(none — distance 0 reached; candidate-ready. Remaining work is integration:
layer-1 → Judge → driver-written canonical-asm grant for the two cop2 islands
(cluster ruling, cop2-addressing-preamble-cluster.md:73) → verify-oracle.)
