# Hypothesis ledger — func_8006E49C

## s2 (structural, 2026-07-17)
- H1 accumulator-respelling-closes-at-0: KILLED (sandbox 25; sched1 store-sinking + temp-register scatter).
- H2 single-reused-t-serializes-schedule: CONFIRMED (38/38 target insn order; sandbox 28 = pure register-identity diffs).
- H3 return-through-t-gives-v0-pref: KILLED as sufficient (combine folds the final add into $v0 directly; with $v0 occupied by the const, global gives t $v1 regardless of preference).
- H4 short-overlapping-u-evicts-const-in-local-alloc: KILLED on all 6 group-1 segments (sched1 unpins the u store; u becomes the $v1 occupier instead).
- H5 t-plus-equals-chain-single-death: KILLED (cse re-association restores deaths where bytes are right, leaves wrong bytes where it doesn't).
- H6 const-holder-spelling-variants-flip-allocation: KILLED (v1/v3/v7/v17/v18 all const=$v0).
- H7 residual-is-allocation-only: CONFIRMED (diagnostic pin-probe = exact 38/38 match).

## Frontier for s3
- Permuter directed sweep from candidate.c (28, order-exact): PERM_TEMP_FOR_EXPR / PERM_VAR mutations over the temp-assignment structure; the residual is 28 register-identity diffs (permuter weight ~5 each), a clean gradient. Any closing form must be vetted: no dead stores (Judge bar for THIS function bans the dead-assign family outright), no pins, no volatile.
- Open mechanism question: what REAL single-BB C shape makes local-alloc's find_free_reg skip $v0 for the 1-death const pseudo? Requires an overlapping earlier-allocated qty or hard-reg live range in [insn1..21]. All enumerated single-extra-variable shapes measured dead; unexplored: multi-statement respellings of the [4]/[0xB]/advance cluster, group-boundary re-spellings that change which pseudo carries 0x9C40 into group 2.
- If s3+s4 exhaust the pure axes: the original-compile contradiction (its temp was also global yet won $v0) suggests re-examining premises with BB2_QTY_DEBUG (needs the instrumented cc1 REBUILT — the built binary predates the instrumentation; rebuild is allowed reading, NOT a toolchain patch since the debug hooks are already in-tree) to dump qty birth/death/refs on both our form and hypothesized originals.

## [s2] Full accumulator respelling (frontier probe 1) closes at 0
- mechanism: one pseudo = no RA decision
- probe: rewrote body as arg0 += K with direct expression temps; sandbox --disable all
- result: sandbox 25; sched1 clusters independent addius and sinks stores, temps scatter across t2,t3,a2,a3,t0,t1
- verdict: KILLED

## [s2] Single reused local t serializes the store chain to target schedule (frontier probe 3)
- mechanism: WAR deps from t's redefinitions pin each store before the next compute, incl. the [8]-deferral
- probe: single-t body; sandbox + objdump diff
- result: sandbox 28 but 38/38 target instruction order/opcodes/immediates; residual is exactly const<->t register swap
- verdict: CONFIRMED

## [s2] Returning through t gives it a $v0 copy-preference that closes the swap
- mechanism: local/global copy suggestion from (set v0 t)
- probe: t = arg0 + 0x1FB0; return t;
- result: sandbox 28 unchanged; combine folds the add into $v0 directly and const still holds $v0 so t's global preference is blocked
- verdict: KILLED

## [s2] A short single-death u overlapping the const's range evicts const from $v0 in local-alloc's priority pass
- mechanism: local-alloc allocates short high-priority qtys first; find_free_reg conflict pushes const to $v1
- probe: u-split swept over ALL six group-1 segments (u2..u8) + cc1-output positional scorer vs target
- result: 33-35 diffs (vs 28 unsplit) every time: sched1 unpins the split segment's store, u's deferred store extends its range into the return region, u takes $v1, const keeps $v0, t displaced to $a2
- verdict: KILLED

## [s2] t += delta chain keeps t single-death (local-allocable with the $v0 suggestion)
- mechanism: self-referencing redefs keep a pseudo continuously live = 1 death (proven by arg0's own accumulator getting $a0 via local-alloc)
- probe: probe4.c full += chain; cc1 output diff
- result: cse re-associates to a0-based form only within constant-arg0 regions (dropping the t-read, so flow restores deaths) and leaves cross-advance defs t-relative with WRONG immediates (addu $3,$3,0x2198 vs target addiu v0,a0,0x1B58)
- verdict: KILLED

## [s2] Const-holder spelling variants flip the allocation
- mechanism: different pseudo birth/refs/class for the 0x9C40 constant
- probe: named s32 step, u16 step, decl-position, register t, plain-return (v1/v3/v7/v17/v18)
- result: all converge to the same 1-death const pseudo; const=$v0 in every variant
- verdict: KILLED

## [s2] The 28-residual is allocation-only (zero structural distance)
- mechanism: if a diagnostic pin reproduces target exactly, everything but the RA flip is already correct
- probe: tmp-only pin-probe t=$2/step=$3 (never in src) + positional scorer
- result: EXACT 38/38 match (pinprobe.s scores 0)
- verdict: CONFIRMED
