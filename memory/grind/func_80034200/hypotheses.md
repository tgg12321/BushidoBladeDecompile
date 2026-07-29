# Hypothesis ledger — func_80034200

Recon-session (s1) frontier. Honest floor 21 decomposes into 3 orthogonal-looking
components; the first two can be attacked independently, the third is the
residual RA-plateau this function's grind will live and die on.

## H1 — phantom frame -8 arises from a specific live local pattern
- **Mechanism:** GCC 2.7.2's frame-size accounting reserves stack bytes for
  ordinary LIVE locals whose values happen to never be stored (see
  [[phantom-frame-slots-gcc272]]). Target has `addiu $sp,-8` / `addiu $sp,8`
  and 0 stores between them; clean C emits no frame. Something in the target's
  C — likely a small handful of scalars that all live entirely in registers
  yet count against `get_frame_size()` — was allocated 8 bytes of stack.
- **Next probe:** compile the clean form with `-da` and read the `.stack` /
  `.greg` dumps to see what allocnos GCC assigned stack slots for; then vary
  the local count/types (e.g. add a `s32 v0_outer;` outside the inner block,
  merge the two `s32 v0` locals, promote the pointer aliases to explicit
  `s16*` walk) to trip GCC into the 8-byte reservation. Compare frame_size
  across variants.
- **Verdict:** OPEN.

## H2 — outer-loop `D_800A389B` reload is CSE-defeat surface
- **Mechanism:** target re-reads `D_800A389B` at outer-loop tail (lui+lbu),
  clean C hoists to a register because CSE sees no intervening writes. The
  `[[split-read-defeats-hoist]]` / `[[store-const-reload-cse]]` family covers
  exactly this shape: inserting a barrier that defeats the hoist (a
  duplicate-read into an arm, or a global-write that could alias the read).
- **Next probe:** try three variants: (a) drop the local `count` and use
  `D_800A389B` in both the entry `<= 0` check and the loop tail directly, no
  intermediate binding; (b) hoist the initial `g_disp_enable = DISP_LOADING`
  store BETWEEN the count-read and the loop, so GCC's alias analysis may
  refuse to CSE across an unrelated global store; (c) if (a) and (b) both
  hoist, re-read D_800A389B via a duplicated statement in an arm that reaches
  the tail unconditionally. Measure sandbox floor after each.
- **Verdict:** OPEN.

## H3 — the residual is a register-rename plateau (target uses arg regs;
  clean C uses t-regs for the accumulator and inner counter)
- **Mechanism:** function is void-void with no calls, so `$a0-$a3` are all
  free from arg-1. Target's `global.c` allocation picked `$a0` for the
  accumulator, `$a3` for outer i, `$a2` for shift; clean-C build picked
  `$t0`/`$a2`/`$a0` respectively — a mirror-image priority-tiebreaker
  outcome. See [[register-alloc-pure-c]] Levers A–D and
  [[call-return-if-result-reuse-v0]].
- **Next probe:** (a) instrumented cc1 `-da` `.greg` dump on the clean form
  to identify each pseudo's `reg_n_refs` / priority and see which reg it
  would be forced to under natural allocation; (b) once identified, try the
  playbook levers — narrow int type (u8 for the byte load path), block-local
  scope for shift/i, and, if H1 opens up phantom-frame territory, use the
  same construct to bias RA at the same time.
- **Verdict:** OPEN — likely the last front to close after H1 and H2.

## Rejected — do NOT re-propose

- **register-asm pins on all locals + `volatile char _pad[8]`** (the
  as-inherited src). Scored 16 by coercing GCC's allocator via
  forbidden-family cheats (`register T x asm("$N")` × 9,
  `dead-vars-local-array` on an unwritten array). Detector-flagged; policy
  is unambiguous under `inline-asm-policy` and `no-new-park-categories`.
  Preserved verbatim in `rejected/register-asm-pins-plus-volatile-pad.c`.

## [s1] as-inherited src used forbidden register-asm pins + volatile-pad array to reach floor 16
- mechanism: 9x `register T x asm("$N")` allocator pins + `volatile char _pad[8]; (void)_pad;` frame reservation — matches inline-asm-policy expanded cheat catalog and no-new-park-categories cheats-by-any-spelling
- probe: compared as-inherited src vs a clean pure-C rewrite; both sandboxed --disable all
- result: cheatful=16, clean=21; the 5-insn delta was entirely the illicit coercion of GCC's allocator and phantom-frame reservation
- verdict: KILLED
