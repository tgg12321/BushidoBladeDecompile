# hypotheses — func_80049584

## Session 1 (recon, 2026-08-17)

### H1 — CONFIRMED. The inherited pins/dummy array hid an ordinary pure-C function; a clean natural-C transcription is closer than the m2c body.
- **mechanism:** the sandbox strips `register asm()` pins and cheat-asm before
  scoring, so the inherited constructs contributed nothing to the 26; the m2c
  transcription's artificial variable structure (one `var_s0` reused for
  everything, sub-word temporaries, `(u32)(~x) >> 31` written literally) is
  what was costing insns.
- **probe:** rewrote the body as plain C — two `do/while` loops with named
  pointer walks, `>= 0` sign comparison, an ordered (lo,hi) normalization,
  `goto end` for the sentinel short-circuit — and ran
  `sandbox func_80049584 --disable all`.
- **result:** floor 26 -> 23, target/build insn counts equal at 99.

### H2 — KILLED. The 8-byte frame delta needs its own lever (phantom-slot / written-never-read local array).
- **mechanism:** [[phantom-slot-frame-lever]] — target reserved `sp,-0x28` vs
  our `-0x20`, the classic symptom of locals GCC pays off in `alter_reg`; the
  inherited body faked it with `s32 dummy[2]`.
- **probe:** deferred adding any frame construct and instead fixed the register
  allocation first (H3), then re-read the prologue/epilogue in the sandbox
  disassembly.
- **result:** the frame delta VANISHED as a side effect of H3 — once the counter
  pseudo crossed the call, the extra call-saved register had to be saved and
  the frame grew to 0x28 on its own. There is no independent frame problem
  here, and no frame-coercion construct is warranted. Do not reach for
  `dead-vars-local-array` or the phantom-slot lever on this function.

### H3 — CONFIRMED (and the load-bearing finding). The loop counter and the computed total are ONE variable in the original source.
- **mechanism:** GCC 2.7.2 `global.c` allocno allocation only makes a pseudo
  eligible for a call-saved hard reg when that pseudo is live across a CALL.
  Neither `0..0x3A` loop counter crosses a call, so no separate counter
  variable can ever land in `$s0` — yet target's counters ARE `$s0`, the same
  register that later holds the `func_8004954C` result. The only C-level shape
  that produces this is a single variable serving both roles.
- **probe:** merged them (`i = func_8004954C(...)`, `i = 0x24/0x88` on the
  sentinel path, `i` read at the `end:` join); measured. Then re-split them as
  a control and measured again.
- **result:** merged 23 -> 16; the control re-split cost 0 -> 12, with all 12
  residual insns being `i` relocating from `$s0` to `$a2` plus the prologue
  reschedule that follows. Confirmed in both directions.
  Rejected control form banked at `rejected/separate-total-var.c`.

### H4 — CONFIRMED. Loop B walks a DIFFERENT pointer variable than loop A.
- **mechanism:** one C variable = one allocno = one hard reg for the whole
  function under `global_alloc`. Target puts loop A's source pointer in `$a2`
  and loop B's pointer in `$v1` — two hard regs, hence two pseudos. Sharing
  one `src` variable across both walks forces a single allocno and cascades a
  mis-assignment through the whole `$a0/$a1/$a2/$v1` cluster.
- **probe:** declared a third pointer `s16 *p` for loop B, left everything else
  untouched, re-ran the sandbox.
- **result:** 16 -> **0**. Function closed at honest distance 0.

## Lever-exhaustion record for the H3 construct
The reuse in H3 is the ONLY construct in the final diff that a reader could
question. It was not chosen from a menu — it is forced:
1. A separate `total` variable was tried and measured: floor 12, banked as a
   rejected form. Its residual is entirely the counter's register class.
2. No call exists inside either loop, so the counter cannot cross a call by any
   restructuring of the loops themselves.
3. Declaration-order / init-order permutations cannot change the outcome: they
   move a pseudo between call-clobbered registers, they cannot make a pseudo
   call-saved-eligible, because eligibility is decided by `reg_crosses_call`
   (liveness), not by allocation order.
4. Every forbidden alternative that WOULD force `$s0` (a `register asm("$16")`
   pin, a dead store keeping the counter live past the call, an `asm` barrier)
   is both banned and score-inert under the sandbox.

## Frontier for future sessions
Empty — the function reached honest distance 0 this session. If the Judge or a
layer-1 reviewer rejects the H3 reuse, the next axis is NOT a respelling of the
same construct: it is to re-derive whether some genuinely-live post-call use of
the counter exists in the original semantics (e.g. the counter being the same
storage as a value the refresh path consumes), which would make the shared
variable semantically real rather than codegen-motivated.

## Session 2 (recon, 2026-08-17) — no new hypotheses

Session 1 was discarded on a self_vet FORMAT defect (single-line `SCOPE:` quote
required by `tools/grinder/grindlib.py:51`), not on any technical finding. This
session re-applied `candidate.c` to `src/text1b.c`, re-measured
`sandbox --disable all` = **0**, and repaired the self-vet format. H1–H4 above
are unchanged and were not re-probed. Frontier remains empty: the function is at
honest distance 0 and the only open question is the Judge's ruling on the H3
single-variable reuse (sanctioned family "variable reuse for codegen control").
