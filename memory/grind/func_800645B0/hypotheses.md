# Hypotheses — func_800645B0

## Session 1 (2026-08-12, recon) — all four CONFIRMED; function reached honest distance 0

### H1 — CONFIRMED. The target is a natural nested loop, not the goto chain HEAD carried.
- **Mechanism:** HEAD's `loop_inner:` goto form plus two `register asm()` pins
  was steering allocation by hand; the target asm's control flow is a plain
  outer `do { ... i += 4; } while (i < 15)` around an inner
  `do { ... break; } while (j < 4)`.
- **Probe:** rewrite as the natural nested do/while with `break`; delete both pins.
- **Result:** 21 -> 17, and the two callee-save assignments (accumulator $s3,
  mask $s2) snapped to the target's immediately.

### H2 — CONFIRMED. GCC's loop.c is hoisting the constant `1` out of the inner loop, and that hoist is what costs the two extra instructions.
- **Mechanism:** `mask = 1 << idx` gives a single-set loop-invariant pseudo;
  `scan_loop`/`move_movables` (loop.c:532 threshold, movable requires
  `n_times_set == 1`) hoist it into a fresh callee-save $s4, adding a
  save/restore pair. The target keeps `li $v1,1` inside the loop.
- **Probe:** route the constant through a scratch variable that is also
  assigned a genuinely-used loop-variant value later in the same loop (the
  `D_800A3444` read of the bit-set RMW), per [[defeat-licm-hoist-var-reuse]].
- **Result:** 17 -> 10, build_insns 81 -> 78 (exact). The shared pseudo also
  landed in $v1 — the same register the target uses for both roles.

### H3 — CONFIRMED. The $v1/$a0 swap between the scratch value and the inner counter is a reference-count ordering, closable by making the scratch the OR destination.
- **Mechanism:** allocno priority scales with reference count; `D_800A3444 =
  val | mask;` puts the OR result in a fresh temp, whereas the target's
  `or $v1,$v1,$s2` writes in place.
- **Probe:** `val = val | mask; D_800A3444 = val;`.
- **Result:** 10 -> 3; val=$v1 and j=$a0 as in the target, and the OR
  destination matched.

### H4 — CONFIRMED. The final residual is a cc1 first-pass-scheduler tie at the inner-loop top, resolvable by interposing an existing statement.
- **Mechanism:** `addu $s0,$s3,$a0` and `li $v1,1` are both ready at cycle 0
  with equal priority (both feed only the `sllv`). reorg.c steals whichever is
  emitted first into the inner back-edge delay slot and moves the loop label
  past it; the target steals the `addu`, our build stole the `li`.
- **Probe:** an 8-variant statement-ordering sweep
  (`tmp/grind/func_800645B0/s1/sweep.py`).
- **Result:** 3 -> **0** with `j += 1;` placed between `idx = i + j;` and
  `val = 1;`. NEGATIVE sub-results worth keeping: merely swapping the two
  statements' order does NOT flip the tie (still 3); naming the
  `D_800A3444` condition read into its own local (3); two-step
  `mask = val; mask = mask << idx;` (3); `idx = i; idx = idx + j;` (3);
  hoisting `val = 1` above the index computation with an off-by-one index
  correction (7, and 79 insns — strictly worse).

## Frontier
Empty — the function is at honest distance 0 and byte-exact (unmasked diff:
only offset-relative branch addresses). The only remaining work is
INTEGRATION, which is outside a grind session's surface: retire the now-redundant
`regfix.txt:2521` rule (`engine retire func_800645B0`), full-build SHA1 verify,
`queue done`.
