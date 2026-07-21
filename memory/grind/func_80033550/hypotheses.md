# Hypothesis ledger — func_80033550

## s1 (recon)

- H1 "the old form's 4 needed the pin" — **KILLED**: pin-free do-while form
  scores 4; the pin was inert (sandbox had been stripping it anyway).
- H2 "idx spelling (i*3 + s32-array indexing) moves the <<2 or changes RA" —
  **KILLED**: byte-identical output; combine canonicalizes ×12.
- H3 "`register` storage class on the alias biases the pointer home" —
  **KILLED** (no effect, still a1; also stripper-flagged spelling).
- H4 "sched hoist of final sll is a separate honest gap" — **CONFIRMED
  closed**: single-level FAKE-annotated do-while(0) (sanctioned 2026-07-06)
  fences it; floor 5→4.
- H5 (mechanism, CONFIRMED by .greg): a1-vs-a3 is a global.c first-free
  outcome; ptr conflict set = {v0,v1,a0} and w2 shares a1 via death/birth
  adjacency. Flipping to a3 requires making BOTH a1 and a2 conflict with the
  pointer without emitting bytes.

## Live frontier (next sessions)

1. **Permuter campaign** from the floor-4 candidate (directed + random),
   targeting the a1→a3 pointer-home flip. Prior park's un-run avenue; the
   candidate base is better than any prior seed (schedule already matches).
2. **Instrumented cc1 forensics** (BB2_ALLOC_DEBUG / -da on variants): find
   any input shape where local-alloc seats an overlapping pseudo in a1/a2
   (e.g. forms where w2's birth precedes ptr's last use), or a preference
   path to a3. Key question: what deleted-post-RA construct (no-op move
   coalescence) can occupy a1+a2 during ptr's range?
3. **Sanctioned-FAKE family sweep** (last resort, byte-neutrality required):
   staged-value-reused-variable / duplicated-statement-into-arms /
   named-local-fake forms that extend an a1/a2 occupant across the pointer's
   range. Note measured trap: extending i's life across the loads steals v1
   from w0 (target needs w0=v1) — any lever must not disturb w-value homes.

## [s1] The inherited form's floor-4 required the asm("a3") register pin
- mechanism: sandbox strips pins before scoring, so the pin could never have moved the score
- probe: removed the pin, kept the do-while wrap, sandbox --disable all
- result: score 4, cheat_asm_stripped back to file baseline (369) — pin was inert
- verdict: KILLED

## [s1] Respelling idx as i*3 with s32-array indexing changes chain placement or RA
- mechanism: move the <<2 into store-address arithmetic so combine/sched treat the chain differently
- probe: (&D_80107850)[i*3] form, sandbox + objdump diff
- result: byte-identical output to the i*12 form (combine canonicalizes); score 5 without wrap
- verdict: KILLED

## [s1] `register` storage class on a pointer alias biases the pointer home toward a3
- mechanism: REG_USERVAR/register-decl preference in local/global alloc
- probe: register s32 *p = arg0; loads via p; sandbox
- result: still 4 (a1), and the engine stripper counts the register-alias as strippable (369->370) — spelling is also reviewer-hostile
- verdict: KILLED

## [s1] The sched1 hoist of the final sll above the three lw's is honestly closeable
- mechanism: single-level do-while(0) loop notes fence the tail schedule — sanctioned for any codegen effect per the 2026-07-06 owner rewrite of do-while-zero-exception (FAKE-annotated)
- probe: pin-free wrap around {idx = new_var; w0 = arg0[0];}, sandbox
- result: score 4, schedule matches target exactly; residual is purely the 4 pointer-home insns
- verdict: CONFIRMED

## [s1] a1-vs-a3 is a global.c first-free allocation outcome, not scheduling
- mechanism: ptr pseudo 72 conflicts {v0,v1,a0} only; w2 (local, a1) shares a1 via death/birth adjacency at its own lw; first free = a1. Target's a3 requires BOTH a1 and a2 to conflict with ptr
- probe: cc1 -da full dump; read .greg conflict lists + dispositions
- result: confirmed from probe.i.greg: '72 conflicts: 72 73 2 3 4 29', dispositions 72->a1, 76(w2)->a1 shared; no visible a2 occupant exists in target either — original RA input had byte-invisible pressure
- verdict: CONFIRMED
