# Hypothesis ledger — func_800325E0

## H0 [s1, CONFIRMED] Island respelling (Lever 0 of the 2026-08-18 brief)
Statement: the retired chassis's `.word`-spelled, pin-bridged LZCS island is a classifier
artefact; respelling it as the authorized one-block `mtc2`/`swc2` island with the
func_8002E838 `addiu $v0,$sp,0x10` / `addu $t4,$v0,$zero` store preamble and dropping every
pin removes the fake distance.
Probe: cand1 (`"r"(&sp_tmp)` %2 operand) = 33 at 148/149; cand2 (func_8002E838 spelling)
= 29 at 149/149 with every remaining diff a register rename.
Result: CONFIRMED. The `%2`-operand form is banked as
`rejected/island-percent-operand-addressing-score33.c` (one insn short; wrong preamble).

## H1 [s1, CONFIRMED] Scaled volumes reuse pan_L / pan_R
Statement: the distance-scaled volumes are the same variables as the unit pans
(`pan_L = (distance_scale * pan_L) >> 16;`), not fresh `L_scaled`/`R_scaled` locals.
Mechanism: the pseudo passed to `func_8005C650`'s `$a1`/`$a2` is then the pseudo holding the
unit pan, so it takes the arg register throughout (target `$a1`/`$a2`; ours had `$a0`/`$v1`).
Probe: sandbox 29 -> 18, 149/149.
Result: CONFIRMED.

## H2 [s1, CONFIRMED] pan_sign written in place (nor then srl on the same variable)
Statement: `pan_sign = ~(u32)projected_pan; pan_sign >>= 31;` (owner Ruling 4 2026-09-02,
ordinary compound-assignment split) seats pan_sign in `$a3` ahead of distance_scale.
Mechanism: global.c:635-648 priority = floor_log2(refs)*refs/length; the in-place spelling
gives pan_sign 4 refs (nor def, srl use+def, beqz use) -> 8/29, beating distance_scale's
10/63, so it is seated first (`$a3`), pushing distance_scale/dx/arg0 to `$t0/$t1/$t2` and
the divmod `mfhi` scratches to `$t3/$t1`. `.greg` insn 208: `(set (reg/v:SI 7 a3) (not:SI
(reg/v:SI 5 a1)))`.
Probe: sandbox 18 -> 0, 149/149.
Result: CONFIRMED — match. Zero FAKE constructs, zero pins/barriers/aliasing.

## Not needed (recorded so nobody re-derives them)
- The 2026-08-18 brief's Lever 1 (duplicate the scaled-multiply into both swap arms,
  duplicated-statement family) and Lever 2 (block-local split of pan_L) were never reached:
  the plain three-move swap `{ s32 tmp = pan_L; if (pan_sign != 0) { pan_L = pan_R;
  pan_R = tmp; } }` matches once H1+H2 are in place.
- The retired chassis's `do { v0_m = 0x13 - (v1_m >> 1); } while (0);` wrap is unnecessary;
  the func_80032314 tail spelling (`v1_m = v1_m >> 1; ... >> (0x13 - v1_m)`) matches.
- No `if ((s32)dist_sq >= 0)` guard: the target has no bltz before the island.

## Frontier
(empty — candidate-ready at 0. Remaining steps are the driver's: layer-1 cheat-reviewer,
Judge, owner-cluster allowlist line, `verify-oracle --rebuild`, `queue done`.)

## [s1-retry 2026-09-02] Status
H0-H2 unchanged and re-confirmed (sandbox 0, 149/149). The prior discard was paperwork only
(multi-line SCOPE quotes in self_vet.md); self_vet.md rewritten with single-line quotes.
