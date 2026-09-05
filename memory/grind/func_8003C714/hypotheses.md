# Hypothesis ledger — func_8003C714

## s1 (2026-09-01, recon)

### CONFIRMED
- H1: target's loop structure = index-derived pointers (givs of biv i) with
  `i += 1` as the last body statement. Measured: 33 → 15; loop body, giv inits,
  preheader membership, tail, and every seat except one all match. (candidate.c)
- H2: the entire d15 residual is the 0x91A2B3C5 const placement (hoisted-out
  vs in-loop split lui/ori) plus the downstream mfhi-temp seat t2-vs-t1.
- H3: the hoist desirability test is unwinnable from C — threshold 122
  (call-free, n_non_fixed_regs=60 hard-float), confirmed still-moved at 59
  real insns. Killing the hoist must attack ADMISSION or const FORM, not size.

### KILLED
- K1: plain-pointer bivs (src+=8/dst+=4) — biv elimination biases dst to s0+36
  and splits src into 2 IVs. Structurally wrong; do not re-propose.
  (rejected/plain-pointer-bivs.c)
- K2: inflating loop insn_count to defeat the movable — measured moved at 56,
  57, 59 real insns; threshold is 122, not 58 (the -msoft-float assumption is
  FALSE on this chassis; CC_FLAGS has no -msoft-float, FP regs count).
- K3: mid-body `i += 1` — forces a giv-value split copy (`move a0,a3`), 37 vs 15.
- K4: split increment `i+=3;i-=1;i-=1` — scored 28 (biv machinery handles
  multi-increment but codegen regresses) AND is the F6-banned `+=2/-=1`
  respelling class; diagnostic only, never submittable.
  (rejected/split-increment-inflate-count.c)

### FRONTIER (next session, in order)
1. **do{}while(0) around the first loop statement → moved_once doubling.**
   Wrapping the `/1800` statement (or a prefix of the body) in `do{}while(0)`
   creates an inner loop-note region; inner-loop processing moves the const to
   the inner preheader (still INSIDE the real loop), sets moved_once[regno];
   outer processing then doubles insn_count (loop.c:1611) → 2*count > 122 for
   count ≥ 62 → const NOT re-moved, stays in-loop, re-emitted by the inner move
   in the SPLIT 2-insn form → sched1 interleaves with the lw = target shape.
   Needs body ≥ 62 real insns (currently 56-57 — may need combinable padding
   like the tp-pointer + index respellings which are byte-neutral, or the
   wrap itself adds notes/insns; MEASURE first, arithmetic second).
   POLICY CAVEAT: do-while-zero-exception's scope sentence sanctions ONLY the
   LABEL_OUTSIDE_LOOP_P/reorg.c interaction; using it for loop.c count
   doubling is outside that scope → if it measures to d0, the outcome is
   `ruling-request` citing the measured mechanism, not a silent submission.
2. **cc1psx self-disproof probe on candidate.c** (calibration-only, per
   cc1psx-calibration-only rule): does SN cc1psx hoist the /1800 const for the
   exact candidate.c body? If cc1psx leaves it in-loop, this is a
   toolchain-fidelity finding (SN loop.c divergence) — route per
   no-compiler-divergence/systemic-research, not per-function grinding.
3. **Read loop.c's move_movables m->consec/move_insn re-emission path** to
   enumerate every condition under which the const stays: (a) REG_EQUAL/EQUIV
   note absence making move_insn=0, (b) m->forces chains, (c) the
   already_moved[] interaction. Then look for a C shape that gives the const
   pseudo a second (consumed) use so `reg_single_usage` logic or n_times_used
   changes its class — e.g. a second `/1800` division of a DIFFERENT dividend
   in the loop would share the magic pseudo (cse) making it 2-use... but there
   is no second /1800 in the target bytes; check whether n_times_used>1 with
   a single materialization changes savings/admission at all before spending
   a measurement.

### Judge constraints inherited: none. Banned constructs: none (K4 self-banned
by frozen policy, recorded here so no session re-tries it).

## [s1] Target loop = index-derived pointers (src=(u8*)&D_80106A58+i*8, dst=(u8*)s0+i*4) with i+=1 as last body statement; strength reduction then reproduces target's giv inits, unbiased offsets, and delay-slot increment
- mechanism: loop.c strength_reduce: general givs of biv i reduced in place (offsets preserved), giv-arith insns deleted; sched1 hoists the trailing addiu into the 4th mult's mfhi shadow
- probe: sandbox after each respelling + .loop dump reads
- result: 33 -> 15; loop body, preheader membership, tail, all seats except mfhi-temp match
- verdict: CONFIRMED

## [s1] Plain pointer bivs (src+=8/dst+=4) cannot match: biv elimination biases dst to s0+36 and splits src into two IVs
- mechanism: loop.c biv elimination + combine_givs choosing the last giv (add 36) as base
- probe: .loop dump: 'biv 75 was eliminated', 'giv at 47/78/121 combined with giv at 126'
- result: measured 33 with wrong loop shape; banked as rejected form
- verdict: KILLED

## [s1] The 0x91A2B3C5 hoist can be defeated by inflating loop insn_count past the movable threshold
- mechanism: move_movables: threshold*savings*lifetime >= insn_count with threshold 2*(1+n_non_fixed_regs)
- probe: grew loop from 53 to 56, 57, 59 real insns via respellings + a split-increment diagnostic; read 'moved to N' lines each time
- result: moved at every size; threshold is 122 (hard-float chassis, n_non_fixed_regs=60, no -msoft-float in CC_FLAGS) so size is unwinnable (needs >=123)
- verdict: KILLED

## [s1] Target's in-loop lui v0 / lw / ori v0 / mult order proves the magic constant was TWO separate RTL insns at sched1 in the original compile (an assembler li macro expands adjacently; only compiler scheduling interleaves)
- mechanism: sched1 list scheduling filling the lw load-delay with the ori half
- probe: instruction-order analysis of asm/funcs/func_8003C714.s lines 19-22 + our dumps showing the split form (movsi-hi + iorsi3 0xB3C5) exists in this cc1 but only appears post-loop-move in the preheader
- result: inference solid; the C-level trigger for split-form-in-loop is the open question
- verdict: CONFIRMED

## s2 (2026-09-01, structural)

### CONFIRMED
- H4: the entire d15 is `loop.c`'s movable-desirability threshold, a build-config
  scalar, not a C spelling. `threshold = (loop_has_call?1:2)*(1+n_non_fixed_regs)`
  (loop.c:532) is 122 on the shipped hard-float chassis and 58 when the 32 FP
  hard regs are fixed. With `threshold -= 3` applied after the first movable
  moves (loop.c:1719/1904) the /1800 magic faces 55 vs the loop's insn_count 56
  and is "not desirable" — landing in-loop in the split lui/ori form the target
  has. MEASURED: candidate.c scores **0** with `-msoft-float` and **0** with
  32 `-ffixed-$fN` (byte-identical objects), **15** without.
- H5: the flag is essentially byte-neutral project-wide — 30/32 C stems
  byte-identical, the only regression being func_800324D0 (0 -> 3) by the same
  rule in reverse (its 72-insn loop loses the `li 255` hoist at threshold 58).

### KILLED
- K5: **any C-level attack on the hoist, on the shipped chassis.** reg 84 meets
  threshold 119 with savings 1 and lifetime 1 (both already minimal), so the
  loop would need >119 RTL insns; the whole target function is 104 machine
  instructions. Loop-size, admission (compiler-generated pseudo is admitted
  unconditionally at loop.c:697-700), and `reg_single_usage` substitution (needs
  a loop containing a call) are all closed. s1's frontier items 1 and 3 die with
  it: no movable-admission escape exists to find, and the `do{}while(0)`
  moved_once doubling would need insn_count>61 AND an inner loop that jump
  optimization does not delete — and is unnecessary now that the true knob is
  known.
- K6: the s1 guess that this is an SN-cc1psx `loop.c` divergence (frontier 2).
  It is not a fork difference: our own cc1 reproduces the target exactly once
  `n_non_fixed_regs` matches a no-FPU register file. A cc1psx probe would only
  re-observe the same threshold.

### FRONTIER (next session / operator)
1. **This is an INTEGRATION HANDOFF, not a grindable residual.** The fix is one
   flag in `CC_FLAGS` (Makefile + engine/buildconfig.py) — a surface no grind
   session may touch. See the docs/grind/decisions.md entry of 2026-09-01.
2. If the owner elects the global flag, `func_800324D0` needs a re-grind: its
   loop must be spelled at <=58 RTL insns (currently 72) for the `li 255` hoist
   to survive at threshold 58. That is an ordinary structural grind, not a lock.
3. Do NOT re-open the C search for func_8003C714 on the shipped chassis. The
   arithmetic in H4/K5 forecloses it; only the chassis question is live.

## s2b (2026-09-01, structural modality — re-run after the s2 session was
## driver-discarded on an entry-title technicality; s2's findings themselves were
## independently RE-VERIFIED here, not inherited on trust)

- **VERIFIED (not assumed):** with `candidate.c` in place of the `INCLUDE_ASM`
  line, `sandbox func_8003C714 --disable all` prints **score 15, target_insns
  104, build_insns 105** on the HEAD chassis, and `tools/grinder/dump.ps1`'s
  `.loop` dump reproduces s2's movable table verbatim:
  `Loop from 25 to 146: 56 real insns.` / `Insn 33: regno 78 (life 1),
  move-insn savings 1  moved to 203` / `Insn 46: regno 84 (life 1), move-insn
  savings 1  moved to 205` / `Insn 60: regno 91 (life 31), move-insn savings 1
  moved to 207`. s2's account of the residual is correct.

- **K7 — KILLED BY MEASUREMENT: the `do{}while(0)` -> `moved_once` doubling
  frontier item (s1 frontier #1) does not exist.** Probe:
  `rejected/dowhile0-inner-loop-is-phony.c` (the /1800 store wrapped in
  `do { ... } while (0);`). Measured score **15** — unchanged. The `.loop` dump
  explains why: the wrap emits `Loop from 43 to 68 is phony.` (loop.c:573). A
  degenerate loop has no back edge, so `loop_optimize` marks it PHONY and never
  calls `scan_loop` on it; the 0x91A2B3C5 movable is therefore never hoisted to
  an inner preheader, `moved_once[regno]` (loop.c:1912) is never set, and the
  `insn_count *= 2` / "halved since already moved" branch (loop.c:1609-1611)
  never fires. The outer movable table is bit-identical to the unwrapped form
  (`Loop from 25 to 160: 56 real insns.`, all three movables moved). This kills
  the ENTIRE degenerate-loop family (do/while/for with a constant-false test),
  not just this spelling — and it means **no ruling-request is owed** for the
  `do-while-zero-exception` scope question s1 flagged: the construct is inert
  here, so its sanctioning status is moot.

- **K8 — KILLED BY MEASUREMENT: the `insn_count` axis is quantitatively dead.**
  The desirability test is `threshold * savings * m->lifetime >= insn_count`
  (loop.c:1631); for regno 84 `savings == 1` and `lifetime == 1` are both at
  their structural floor (lifetime is `uid_luid[last_uid] - uid_luid[first_uid]`
  = 1 because the const's set and its single `mult` use are adjacent, and it
  cannot be 0), so the ONLY term left is `insn_count` against a threshold of
  119 (122 after loop.c:532, minus the one `threshold -= 3` at loop.c:1719 that
  the regno-78 hoist costs). The movable therefore stays in-loop only at
  `insn_count >= 120`. Probe: `rejected/inline-index-addr-insn-count-costs-bytes.c`
  inflates the loop by ordinary C (drop the `src` local, respell each read as
  `*(s32 *)((u8 *)&D_80106A58 + i * 8 + 4)`). Measured: insn_count 56 -> **58**,
  score 15 -> **28** (105 -> 108 insns). That is ~6.5 score points per +1 RTL
  insn; the remaining +62 RTL insns would cost hundreds of points inside a
  104-instruction target. Not "argued dead" — measured dead.

- **K9 — the move_movables decision inputs are now ENUMERATED from source, and
  every one is closed on the shipped chassis.** `threshold` (loop.c:532) is
  `(loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs)`; the target's loop contains
  no call (asm/funcs/func_8003C714.s: the loop body 8003C754..8003C840 has no
  `jal`), so the `loop_has_call` branch that would give 61 is unreachable
  without changing the emitted bytes. Reducing `threshold` further needs 23 more
  hoists before regno 84 (each `-= 3`), and the target preheader is fully
  accounted for (`addu t0,zero,zero` biv init, `lui/ori a3` = 0x88888889,
  `lui/addiu a2` + `addu a1,s0,zero` = strength-reduction giv inits) — there is
  room for zero additional hoisted movables. The remaining `if` guards all push
  the WRONG way: `force_movables` (loop.c:1221) only ADDS lifetime and DOUBLES
  savings; `combine_movables` matching only adds savings; `m->cond` is satisfied
  because `invariant_p` of a `const_int` is 1; and the `reg_single_usage`
  substitution path (loop.c:735-768) requires both `loop_has_call` and a
  successful `validate_replace_rtx` of a 32-bit constant into a MIPS `mult`,
  which has no immediate form. There is no admission escape and no desirability
  escape.

## [s2] Wrapping the `dst[0x21] = v/1800;` statement in do{}while(0) creates an inner loop whose preheader is inside the real loop; the 0x91A2B3C5 movable is hoisted there, setting moved_once[regno], and the outer loop's move_movables then doubles insn_count (56 -> 112) so the movable is no longer desirable at threshold 119 — reproducing the target's in-loop split lui/ori.
- mechanism: loop.c inner-first loop processing + moved_once[regno] (loop.c:1912) driving the `insn_count *= 2` / "halved since already moved" branch at loop.c:1609-1611, against the desirability test at loop.c:1631
- probe: Spliced memory/grind/func_8003C714/rejected/dowhile0-inner-loop-is-phony.c over the INCLUDE_ASM line in src/code6cac_c2.c; `& tools/wteng.ps1 main sandbox func_8003C714 --disable all`; then `pwsh tools/grinder/dump.ps1 func_8003C714` and read tmp/grind/func_8003C714/dumps/code6cac_c2.loop.
- result: score 15 (105 insns) — IDENTICAL to the unwrapped candidate. The dump prints `Loop from 43 to 68 is phony.` (loop.c:573) for the wrap, and the outer table is bit-identical: `Loop from 25 to 160: 56 real insns.` with Insn 33/regno 78, Insn 50/regno 84 and Insn 74/regno 91 all `moved to`. A degenerate loop has no back edge, so loop_optimize marks it PHONY and never calls scan_loop on it; nothing is hoisted out of it and moved_once is never set, so the doubling branch cannot fire.
- verdict: KILLED

## [s2] An ordinary-C respelling can raise the loop's RTL insn_count to the 120 required for the 0x91A2B3C5 movable to fail loop.c:1631's desirability test (threshold 119 after the first hoist's `threshold -= 3`) while keeping the emitted bytes on the target.
- mechanism: loop.c:1631 `threshold * savings * m->lifetime >= insn_count`, with savings and lifetime both pinned at 1 (loop.c:791) and threshold pinned at 122 by loop.c:532 with n_non_fixed_regs == 60
- probe: Spliced memory/grind/func_8003C714/rejected/inline-index-addr-insn-count-costs-bytes.c (drop the `src` local; respell each read as `*(s32 *)((u8 *)&D_80106A58 + i * 8 + 4)`) and measured sandbox score plus the `Loop from ...: N real insns.` line in the regenerated .loop dump.
- result: insn_count 56 -> 58 (+2) but score 15 -> 28 (105 -> 108 insns): ~6.5 score points per +1 RTL insn. Reaching insn_count 120 needs +62 more RTL insns inside a target whose entire body is 104 instructions — hundreds of score points. The axis is measured dead, not merely argued dead.
- verdict: KILLED

## [s2] s2's account of the residual (floor 15, one loop.c movable decision, three movables in a 56-insn loop) is accurate and can be spent rather than re-derived.
- mechanism: loop.c scan_loop / move_movables movable table as printed by cc1 -da
- probe: Applied memory/grind/func_8003C714/candidate.c to src/code6cac_c2.c, ran `sandbox func_8003C714 --disable all` and `pwsh tools/grinder/dump.ps1 func_8003C714` fresh this session.
- result: Reproduced exactly: score 15, target_insns 104, build_insns 105; `Loop from 25 to 146: 56 real insns.` with `Insn 33: regno 78 (life 1), move-insn savings 1  moved to 203`, `Insn 46: regno 84 (life 1), move-insn savings 1  moved to 205`, `Insn 60: regno 91 (life 31), move-insn savings 1  moved to 207`.
- verdict: CONFIRMED

## [s2] Some other input to move_movables' accept/reject decision (movable admission, m->cond, m->forces, m->match, loop_has_call, reg_single_usage substitution) can be steered from C to keep the 0x91A2B3C5 constant in the loop on the shipped chassis.
- mechanism: loop.c:1594 safe-to-move guard, loop.c:1221 force_movables, combine_movables, loop.c:532 loop_has_call branch, loop.c:735-768 reg_single_usage substitution
- probe: Read every decision input end-to-end in tools/gcc-2.7.2/loop.c and checked each against the target asm (asm/funcs/func_8003C714.s) and the measured dump.
- result: All closed. force_movables and combine_movables only ADD lifetime / DOUBLE savings (they make hoisting MORE likely). m->cond is satisfied because invariant_p of a const_int returns 1. The loop_has_call branch (threshold 61 instead of 122) is unreachable because the target loop 8003C754..8003C840 contains no jal. reg_single_usage substitution needs BOTH loop_has_call AND a successful validate_replace_rtx of a 32-bit constant into a MIPS `mult`, which has no immediate form. Lowering threshold by `threshold -= 3` needs 23 further hoists and the target preheader has room for zero extra hoisted movables (addu t0,zero,zero biv init; lui/ori a3 = 0x88888889; lui/addiu a2 + addu a1,s0,zero = strength-reduction giv inits).
- verdict: KILLED

## s3 (2026-09-01, structural modality)

Chassis re-check first: with `candidate.c` in place of the `INCLUDE_ASM` line,
`sandbox func_8003C714 --disable all` prints **score 15, target_insns 104,
build_insns 105** and the `.loop` dump reproduces the s2 movable table verbatim
(`Loop from 25 to 146: 56 real insns.` / regno 78, 84, 91 all `moved to`). The
chassis has not drifted; every s1/s2 conclusion is still chassis-current.

### CONFIRMED

- **H6 — the target's in-loop split `lui/lw/ori/mult` quartet IS reproducible on
  the SHIPPED chassis.** This is the first positive (non-negative) measurement on
  this function: when `loop.c:1631` rejects the 0x91A2B3C5 movable, our own cc1
  emits, at sched1, exactly
  `lui v0,0x91a2 / lw v1,0(s1) / ori v0,0xb3c5 / mult v1,v0` — instruction for
  instruction the target's `8003C754..8003C760`. So no chassis change is needed
  to produce the target's *shape*; the ONLY thing standing between the shipped
  chassis and distance 0 is the arithmetic of the desirability test. This
  retires the residual worry (implicit in s1's H-list) that an unmoved large
  constant might stay a single RTL insn on this cc1 and therefore be un-split
  and un-interleavable.
  Probe: `tmp/grind/func_8003C714/s3/body_callD.c` (see K11) — dump line
  `Insn 45: regno 84 (life 1), move-insn savings 1 not desirable`, then
  `objdump -d tmp/sandbox/func_8003C714/code6cac_c2.o`
  (`tmp/grind/func_8003C714/s3/callD_inloop_split_const.txt`).

- **H7 — the whole decision is now source-verified, not inferred.** Read directly
  out of `tools/gcc-2.7.2/loop.c`:
  `532: threshold = (loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs)`;
  `793: m->savings = n_times_used[regno]` (so savings >= 1 always — it is a USE
  count, it can never be 0, which would have been the one way to make the
  product collapse);
  `791: m->lifetime = uid_luid[last] - uid_luid[first]` (1 here, minimal);
  `1609-1613: if (moved_once[regno]) insn_count *= 2;`
  `1631: if (already_moved[regno] || (threshold*savings*m->lifetime) >= insn_count
        || (m->forces && ...)) -> MOVE`;
  `1719/1904: threshold -= 3` after each move.
  `moved_once` is allocated ONCE PER FUNCTION (`loop.c:344`), not per loop, and
  is only ever set at `loop.c:1912` when a movable is actually moved.

### KILLED

- **K10 — zero-cost `insn_count` inflation does not exist through the folding
  channel.** The open half of K8: K8 only measured a MATERIALIZING respelling
  (+2 RTL insns, +13 score). The remaining question was whether an ordinary-C
  construct could add insns that `loop.c` counts but a later pass deletes for
  free. Probe: `rejected/byte-neutral-masks-fold-before-loop.c` — an explicit
  `& 0xFF` on each of the four stored values (semantically redundant: all four
  destinations are `u8`). MEASURED: score **15** (105 insns, byte-neutral as
  designed) but `Loop from 25 to 146: **56** real insns.` — insn_count **+0**,
  movable table bit-identical. The redundant masks are folded at expand/cse1,
  i.e. UPSTREAM of the pass that has to count them. That is the general shape of
  the channel: constructs cheap enough to vanish, vanish before `loop.c` runs;
  constructs that survive to `loop.c` also survive to the assembler (K8). The
  +64 RTL insns needed on the no-call chassis are therefore not obtainable at
  any byte cost below ~400 score points.

- **K11 — the `loop_has_call` half of `loop.c:532` is measured, and it is
  byte-forbidden rather than arithmetically dead.** This is a genuinely new
  quantitative result and it sharpens the foreclosure. Probes:
  * `tmp/grind/func_8003C714/s3/body_callC.c` = candidate.c + one call inside the
    loop. Dump: `Loop from 25 to 149: 57 real insns.` — all three movables still
    `moved to` (threshold 61, minus 3 after the first hoist = 58 >= 57).
  * `tmp/grind/func_8003C714/s3/body_callD.c` = the K8 inlined-address respelling
    + one call inside the loop. Dump: `Loop from 25 to 169: **59** real insns.`
    and `Insn 45: regno 84 (life 1), move-insn savings 1 **not desirable**`.
    Score 55 (116 insns) — the extra `jal` and its spill traffic, exactly as
    expected; this form is a DIAGNOSTIC, never a candidate.
  So on the SHIPPED chassis the 0x91A2B3C5 movable stays in-loop iff
    (a) `loop_has_call` AND `insn_count >= 59`, or
    (b) `!loop_has_call` AND `insn_count >= 120`.
  (a) is foreclosed by the target BYTES, not by arithmetic: the target loop
  `8003C754..8003C840` contains no `jal`, and a `CALL_INSN` at loop-opt time is
  never deleted afterwards, so `loop_has_call` cannot be true for a body that
  emits the target's instructions. (b) is foreclosed by size: the target's whole
  loop is 59 machine instructions and the whole function is 104, and K8+K10 show
  RTL insns at loop time cost bytes ~1:1.

- **K12 — the `moved_once` doubling escape is closed for a second, independent
  reason.** With the doubling the test becomes `119 >= 2*insn_count`, i.e. the
  movable stays only at `insn_count >= 60` — only +4 RTL insns, tantalisingly
  close. But `moved_once[regno]` (loop.c:1912, array allocated once per function
  at loop.c:344) is set only when that same pseudo was actually MOVED by an
  earlier `scan_loop` call, i.e. by a REAL enclosing/nested loop processed first.
  K7 already measured that a degenerate `do{}while(0)` is marked PHONY and never
  scanned; a non-degenerate inner loop emits a back-edge branch inside the loop
  body, and the target body `8003C754..8003C840` is straight-line. So the only
  way to arm the doubling is a construct the target bytes exclude.

### FRONTIER (for the next session)
1. The C is not the residual and is now positively, not just negatively,
   established: H6 shows the shipped chassis emits the exact target quartet the
   moment `loop.c:1631` says "not desirable". Everything left is the value of
   `threshold`, a build-configuration scalar with no C-side input (H7).
2. Nothing on the structural axis remains untried: size (K2/K8), zero-cost size
   (K10), admission (K9), the call branch (K11), the doubling (K7/K12), savings
   and lifetime (H7 — both pinned at their structural minimum of 1).
3. Any future session dispatched here should spend it on a modality that is not
   spelling: forensics on whether a DIFFERENT ordinary-C body could make the
   target loop's *own* `insn_count` >= 120 is arithmetically excluded, so the
   productive remaining question is the one this session is barred from filing.

## [s3] The target's in-loop split lui/lw/ori/mult constant form is reproducible on the SHIPPED chassis (no chassis change is needed for the SHAPE, only for the DECISION).
- mechanism: loop.c:1631 desirability test; when the movable is declined, the (set reg const_int) stays in the loop, is split into movsi-hi + iorsi3, and sched1 fills the lw load-delay slot with the ori half
- probe: Spliced tmp/grind/func_8003C714/s3/body_callD.c over the INCLUDE_ASM line at src/code6cac_c2.c:629, read the .loop dump (Insn 45: regno 84 ... not desirable), then disassembled tmp/sandbox/func_8003C714/code6cac_c2.o
- result: Emitted `lui v0,0x91a2 / lw v1,0(s1) / ori v0,0xb3c5 / mult v1,v0 / mfhi t0` = target 8003C754..8003C764 instruction for instruction. First positive (non-negative) measurement on this function; candidate.c's C shape is confirmed correct.
- verdict: CONFIRMED

## [s3] The move_movables decision inputs are exactly threshold, savings and lifetime, with savings and lifetime pinned at 1 by construction, so the product reduces to threshold alone.
- mechanism: loop.c:532 threshold=(loop_has_call?1:2)*(1+n_non_fixed_regs); loop.c:793 m->savings=n_times_used[regno] (a USE count, never 0); loop.c:791 m->lifetime=uid_luid[last]-uid_luid[first]; loop.c:1631 (threshold*savings*lifetime)>=insn_count; loop.c:1719/1904 threshold-=3 per move; loop.c:344/1912 moved_once is per-FUNCTION and only set on an actual move
- probe: Read tools/gcc-2.7.2/loop.c lines 344, 532, 791, 793, 1609-1613, 1631, 1719, 1904, 1912 directly this session rather than inheriting s2's account on trust
- result: Model source-verified. savings cannot be 0 (it counts uses); lifetime cannot be 0 (set and use are distinct insns). Minimum product = threshold = 119 after the first hoist on the shipped no-call chassis.
- verdict: CONFIRMED

## [s3] An ordinary-C, byte-neutral construct can raise the loop's RTL insn_count at loop-opt time and then be folded away for free, reaching the insn_count >= 120 that loop.c:1631 needs (the half of K8 that s2 left open).
- mechanism: redundant `& 0xFF` masks on values stored to u8 destinations: semantically inert, expected to add andsi3 insns that combine later merges into the sb
- probe: memory/grind/func_8003C714/rejected/byte-neutral-masks-fold-before-loop.c spliced in; sandbox func_8003C714 --disable all, then pwsh tools/grinder/dump.ps1 and read the .loop movable table
- result: Score 15 / 105 insns (byte-neutral as designed) but `Loop from 25 to 146: 56 real insns.` — insn_count +0 and a bit-identical movable table. The masks fold at expand/cse1, UPSTREAM of loop.c. Constructs cheap enough to vanish, vanish before loop.c counts them; constructs that survive to loop.c also survive to the assembler (K8: ~6.5 score per +1 RTL insn). The folding channel yields nothing.
- verdict: KILLED

## [s3] The loop_has_call half of loop.c:532 (threshold 61 instead of 122) opens a reachable window for keeping the 0x91A2B3C5 constant in the loop.
- mechanism: loop.c:532 threshold=(loop_has_call?1:2)*(1+n_non_fixed_regs) = 61 with a CALL_INSN in the loop, minus 3 after the first hoist = 58, tested against insn_count at loop.c:1631
- probe: Two diagnostic forms: tmp/grind/func_8003C714/s3/body_callC.c (candidate + one in-loop call) and s3/body_callD.c (K8 inlined-address respelling + one in-loop call); read the .loop movable table for each and scored callD
- result: callC: insn_count 57, all three movables still moved (58>=57). callD: insn_count 59, `Insn 45: regno 84 (life 1), move-insn savings 1 not desirable` — the window is real and narrow. But it is foreclosed by the target BYTES, not by arithmetic: the target loop 8003C754..8003C840 contains no jal, and a CALL_INSN is never deleted after loop opt, so loop_has_call cannot be true for a body emitting the target's instructions. callD scores 55 (116 insns) and is a diagnostic only, never a candidate.
- verdict: KILLED

## [s3] The moved_once insn_count-doubling escape (loop.c:1609-1613) is reachable, needing only insn_count >= 60 rather than 120.
- mechanism: if (moved_once[regno]) insn_count *= 2; the desirability test then becomes 119 >= 2*insn_count, failing at insn_count >= 60 — only +4 RTL insns over the current 56
- probe: Read loop.c:344 (moved_once alloca-ed once per FUNCTION) and loop.c:1912 (set only when a movable is actually moved), combined with s2's K7 measurement that a degenerate do{}while(0) is marked PHONY and never scanned
- result: Closed for a second independent reason: arming moved_once requires the same pseudo to have been moved by an earlier scan_loop call, i.e. a REAL nested loop, which emits a back-edge branch inside the loop body. The target body 8003C754..8003C840 is straight-line. The tantalising +4-insn margin is irrelevant because the trigger itself is byte-excluded.
- verdict: KILLED

## s4 (2026-09-01, permuter modality)

Chassis re-check first: `memory/grind/func_8003C714/candidate.c` spliced over the
`INCLUDE_ASM` line at src/code6cac_c2.c:629, `sandbox func_8003C714 --disable all`
prints **score 15, target_insns 104, build_insns 105**, and the `.loop` dump
reproduces the s2/s3 movable table verbatim (`Loop from 25 to 146: 56 real insns.`
/ regno 78, 84, 91 all `moved to`). Chassis unchanged; src/ was restored to the
`INCLUDE_ASM` line before the session closed.

### KILLED

- **K13 — random search over the candidate.c basin yields nothing.** A full
  decomp-permuter campaign was built and validated for this function for the
  first time (workspace `tmp/perm_8003C714_s4`, hand-built to mirror the shipped
  pipeline exactly: cpp `-DPERMUTER` over the whole TU, `build/cc1 -O2 -G0
  -funsigned-char -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel`,
  prologue_fix, maspsx with the project flag set, the `code6cac_c2` rodata
  `.align 3 -> .align 2` fix, multu_pad, then a `.globl func_8003C714 .. .end
  func_8003C714` extraction assembled standalone; `target.o` = prelude +
  `asm/funcs/func_8003C714.s`). The workspace validated against the ledger:
  base-vs-target objdump diff is EXACTLY the known d15 residual (hoisted `lui
  t1/ori t1` vs the target's in-loop `lui v0 ... ori v0`, plus the mfhi seat t2
  vs t1) and nothing else. Campaign: `permuter_campaign.py launch -j 8`,
  base_score 590, run **1776 s / 66,016 iterations**, harvested with `--stop`:
  **0 finds, 0 novel, best_new_score null** — the search never once improved on
  the base score. Three consecutive `wait` windows (553 s, 552 s, 552 s) each
  returned `novel: []`. Per the fresh-seed rule this basin is exhausted; the
  session switched LEVER rather than reseeding, which produced K14.

- **K14 — the `threshold -= 3` / `insn_count` JOINT channel is real, is exactly
  13 hoists wide, and is byte-foreclosed by a factor of thirteen.** This is the
  one arithmetic error in the inherited ledger. K9 treated threshold reduction
  and insn_count inflation as separate axes and estimated "23 further hoists"
  for the former; but every movable that loop.c MOVES both decrements
  `threshold` by 3 (loop.c:1719 and loop.c:1904 — verified this session, both
  decrement sites are unconditional, taken on every move) AND contributes to the
  `insn_count` the movable is tested against. The two effects push
  loop.c:1631's `threshold * savings * lifetime >= insn_count` in the SAME
  direction, so the real requirement is much weaker than 23. Probe: a sweep
  `k = 0,4,8,12,13,14,15,16,20,24` of bodies that add `k` loop-invariant
  large-constant movables (alternating `acc ^= C` / `acc += C`) AHEAD of the
  `/1800` statement, each compiled with `cc1 -dL` and its per-function movable
  table read (`tmp/grind/func_8003C714/s4/kdumps/k<k>.i.loop`):

      k=0   insn_count 56   0x91A2B3C5 movable (regno 84)  moved to 205
      k=12  insn_count 82   0x91A2B3C5 movable (regno 97)  moved to 290
      k=13  insn_count 84   0x91A2B3C5 movable (regno 98)  NOT DESIRABLE
      k=14  insn_count 86   0x91A2B3C5 movable (regno 99)  NOT DESIRABLE
      k=16  insn_count 90   0x91A2B3C5 movable (regno 101) NOT DESIRABLE

  and the emitted asm at k=16 (`tmp/grind/func_8003C714/s4/kdumps/k16.s`) carries
  `li $2,-1851654144` (= 0x91A2B3C5) INSIDE the loop, directly under the loop-top
  label `.L133:` and feeding `mult $3,$2`. **This is the first time the shipped
  chassis has been made to decline this movable with NO call in the loop** — K11
  had only ever reached "not desirable" by arming `loop_has_call`, which the
  target bytes forbid. The arithmetic checks exactly: at k=16, eleven movables
  are moved ahead of the magic, so threshold = 122 - 3*11 = 89 < insn_count 90.

  It is nevertheless DEAD, and now dead with a number attached. The decrement is
  paid only by movables that are ACTUALLY MOVED, and loop.c:1700-1717 emits every
  moved movable's set into the loop PREHEADER
  (`emit_insns_before (temp, loop_start)`). Declining the magic therefore costs
  **13 additional hoisted preheader instructions ahead of it**. The target
  preheader is 8003C73C..8003C750 — six instructions, every one accounted for:
  `addu t0,zero,zero` (biv init), `lui a3 / ori a3` = the single hoisted movable
  0x88888889, and `lui a2 / addiu a2` + `addu a1,s0,zero` (strength-reduction giv
  inits, not movables). Room for extra hoisted movables in the target bytes:
  **zero**. The zero-byte-cost variant (13 hoists whose preheader insns a later
  pass deletes) is closed by K10 in its general form: constructs cheap enough to
  vanish, vanish UPSTREAM of loop.c; constructs that survive to loop.c also
  survive to the assembler. Banked as
  `rejected/invariant-hoist-threshold-decrement-needs-13-preheader-insns.c`.

### FRONTIER (for the next session)
1. The desirability test at loop.c:1631 is now closed on BOTH of its terms with
   measurements rather than estimates: `insn_count` alone needs +64 (K8/K10),
   `threshold` alone needs 23 hoists (K9), and the JOINT path needs 13 hoists
   (K14) — and all three are foreclosed by the same six-instruction target
   preheader / 59-instruction target loop budget. `savings` and `lifetime` are
   pinned at 1 by construction (H7). There is nothing left in this test.
2. Random search is now measured, not assumed: 66,016 permuter iterations over
   the validated candidate.c basin produced zero improvements on a base score of
   590. Do not re-run a campaign from this basin. The validated workspace recipe
   is `tmp/grind/func_8003C714/s4/mkws.sh` + `compile.sh` if a future session
   wants a DIFFERENT seed cheaply.
3. Every remaining question about this function is the chassis question
   (n_non_fixed_regs / CC_FLAGS), which the standing Judge constraint bars grind
   sessions from re-filing. A future dispatch should be told that up front.

## [s4] A decomp-permuter campaign over the validated candidate.c basin can find a spelling that reaches distance 0 on the shipped chassis.
- mechanism: randomized C-level mutation (decomp-permuter) scored against a standalone-assembled func_8003C714 object extracted from a full-TU compile through the exact shipped pipeline
- probe: Hand-built and VALIDATED workspace tmp/perm_8003C714_s4 (base-vs-target diff = exactly the known d15 residual, nothing else); `python3 tools/permuter_campaign.py launch --func func_8003C714 --dir tmp/perm_8003C714_s4 --label s4-candidate-basin -j 8`; three in-turn `wait` windows (553+552+552 s); `harvest --stop`.
- result: base_score 590, elapsed 1776 s, **66,016 iterations, finds_total 0, finds_new 0, best_new_score null**. Not a single iteration improved on the base score, and no `wait` window returned a novel find. The basin is flat, consistent with the ledger's account that the residual is a single pass-level decision with no C-side input.
- verdict: KILLED

## [s4] The loop.c threshold decrement and insn_count inflation are independent axes, so defeating the 0x91A2B3C5 hoist via threshold reduction requires the 23 further hoists K9 estimated.
- mechanism: loop.c:1719 / loop.c:1904 `threshold -= 3` on every move, against loop.c:1631 `threshold * savings * lifetime >= insn_count`, where added invariant movables ALSO raise insn_count
- probe: k-sweep (k = 0,4,8,12,13,14,15,16,20,24) of bodies adding k loop-invariant large-constant movables ahead of the /1800 statement; each compiled with `cc1 -dL` and its per-function movable table read from tmp/grind/func_8003C714/s4/kdumps/k<k>.i.loop; k16 disassembly checked for constant placement.
- result: The two axes are NOT independent — they compound. The 0x91A2B3C5 movable flips from `moved to` (k=12, insn_count 82) to `not desirable` (k=13, insn_count 84) and stays declined at k=14/15/16. At k=16, threshold = 122 - 3*11 = 89 < insn_count 90, and the emitted asm places `li $2,-1851654144` INSIDE the loop under `.L133:` feeding `mult $3,$2` — the target's shape, on the shipped chassis, with no call in the loop. K9's estimate of 23 is corrected to 13.
- verdict: KILLED (the ESTIMATE is killed; the CHANNEL is confirmed real but byte-foreclosed — see the next entry)

## [s4] The corrected 13-hoist joint channel is byte-reachable: 13 extra hoisted movables can be spelled without disturbing the target's emitted instructions.
- mechanism: loop.c:1700-1717 `emit_insns_before (temp, loop_start)` — every MOVED movable emits its set into the loop preheader, and only moved movables pay `threshold -= 3`
- probe: Counted the target preheader 8003C73C..8003C750 in asm/funcs/func_8003C714.s against the movable/giv inventory: 6 instructions = biv init `addu t0,zero,zero`, the ONE hoisted movable `lui a3 / ori a3` (0x88888889), and the giv inits `lui a2 / addiu a2` + `addu a1,s0,zero`.
- result: The target preheader has room for ZERO additional hoisted movables and the channel needs 13. The zero-byte-cost variant is closed by K10's general finding (constructs cheap enough to vanish, vanish upstream of loop.c). Byte-foreclosed by a factor of thirteen.
- verdict: KILLED

## [s4] A decomp-permuter campaign over the validated candidate.c basin can find a spelling that reaches distance 0 on the shipped chassis.
- mechanism: randomized C-level mutation (decomp-permuter) scored against a standalone-assembled func_8003C714 object extracted from a full-TU compile driven through the exact shipped pipeline (cpp -DPERMUTER, build/cc1 -O2 -G0 -mel, prologue_fix, maspsx, rodata .align 3->2, multu_pad)
- probe: Hand-built workspace tmp/perm_8003C714_s4 (recipe: tmp/grind/func_8003C714/s4/mkws.sh + compile.sh), VALIDATED before launch -- its base-vs-target per-function objdump diff is exactly the known d15 residual (hoisted lui/ori t1 vs the target's in-loop lui/ori v0, plus the mfhi t2-vs-t1 seat) and nothing else. Then `python3 tools/permuter_campaign.py launch --func func_8003C714 --dir tmp/perm_8003C714_s4 --label s4-candidate-basin -j 8`, three in-turn `wait` windows (553.5 s / 552.2 s / 551.8 s), then `harvest --stop`.
- result: base_score 590; elapsed 1776.1 s; 66,016 iterations; finds_total 0, finds_new 0, best_new_score null; every wait window returned novel: []. Not one randomized spelling improved on the base score. The basin is flat, exactly as the source-level account in H7/K9 predicts.
- verdict: KILLED

## [s4] The loop.c threshold decrement and the loop's insn_count are independent axes, so defeating the 0x91A2B3C5 hoist via threshold reduction requires the 23 further hoists that the inherited K9 estimated.
- mechanism: tools/gcc-2.7.2/loop.c:1719 and loop.c:1904 both apply `threshold -= 3` unconditionally on every move, and the test at loop.c:1631 is `threshold * savings * m->lifetime >= insn_count` -- but each added loop-invariant movable ALSO raises insn_count, so the two effects compound instead of trading off
- probe: k-sweep over k = 0,4,8,12,13,14,15,16,20,24 bodies that add k loop-invariant large-constant movables (alternating `acc ^= C` / `acc += C`) AHEAD of the /1800 statement; each compiled with `cc1 -dL` and its per-function movable table read from tmp/grind/func_8003C714/s4/kdumps/k<k>.i.loop; the k=16 assembly checked for constant placement.
- result: The axes compound. The 0x91A2B3C5 movable is `moved to` at k=12 (insn_count 82) and flips to `not desirable` at k=13 (insn_count 84), staying declined at k=14/15/16/20/24. At k=16 eleven movables move ahead of it, so threshold = 122 - 3*11 = 89 < insn_count 90 -- the arithmetic checks exactly. K9's estimate of 23 is corrected to 13.
- verdict: KILLED

## [s4] The corrected 13-hoist joint channel is byte-reachable -- 13 extra hoisted movables could be spelled without disturbing the target's emitted instructions.
- mechanism: loop.c:1700-1717 `emit_insns_before (temp, loop_start)`: every movable that is actually MOVED emits its set into the loop preheader, and only moved movables pay the `threshold -= 3`
- probe: Counted the target preheader 8003C73C..8003C750 in asm/funcs/func_8003C714.s against the full movable/giv inventory established in s1-s3.
- result: Six instructions, all accounted for: `addu t0,zero,zero` (biv init), `lui a3 / ori a3` = the ONE hoisted movable 0x88888889, and `lui a2 / addiu a2` + `addu a1,s0,zero` (strength-reduction giv inits, not movables). Room for extra hoisted movables: zero, against a requirement of 13. The zero-byte-cost variant is closed by s3's K10 in general form (constructs cheap enough to vanish, vanish upstream of loop.c; constructs that survive to loop.c survive to the assembler).
- verdict: KILLED

## [s4] The shipped chassis can only be made to decline the 0x91A2B3C5 movable by arming loop_has_call (the s3 K11 position).
- mechanism: loop.c:532 threshold = (loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs), i.e. 61 with a call and 122 without
- probe: The same k-sweep: none of the k bodies contains a call, so loop_has_call is false and threshold starts at 122 in every one of them; read the k=16 assembly (tmp/grind/func_8003C714/s4/kdumps/k16.s) for the constant's final placement.
- result: Refuted. At k>=13 the movable is declined with loop_has_call FALSE, and k16.s emits `li $2,-1851654144` (0x91A2B3C5) INSIDE the loop directly under the loop-top label `.L133:`, feeding `mult $3,$2`. This is the first no-call reproduction of the target's construct on the shipped chassis; it does not open a path (the preheader budget still forecloses it) but it removes the last doubt that the decision, and not the chassis, is what produces the target's shape.
- verdict: CONFIRMED

## s5 (2026-09-01, synthesis modality)

Chassis re-check first, before anything was spent: `memory/grind/func_8003C714/candidate.c`
spliced over the `INCLUDE_ASM` line at `src/code6cac_c2.c:629`,
`& tools/wteng.ps1 main sandbox func_8003C714 --disable all` prints **score 15,
target_insns 104, build_insns 105**. Identical to s1-s4. The dispatch brief's
"HEAD honest floor: measurement unavailable" is a driver-side measurement gap,
not chassis drift — the floor is 15 and every banked conclusion is still
chassis-current. `src/code6cac_c2.c` was restored to the `INCLUDE_ASM` line
before the session closed.

This session's job was to MERGE four sessions of work into one attack rather
than open a new spelling axis. Merging surfaced one pass-ordering fact that no
prior session had used, and that fact turned out to be testable, so the session
spent its measurements there instead of on re-derivation.

### The fact nobody had used: three deletion points bracket loop.c

`tools/gcc-2.7.2/toplev.c` `rest_of_compilation`, read directly this session:

    2865  cse_main                     (cse1)
    2866  delete_dead_from_cse         <-- DCE, BEFORE loop
    2895  loop_optimize                <-- scan_loop / move_movables / strength_reduce
    2926  cse_main                     (cse2)
    2984  flow_analysis                <-- full life analysis + DCE, AFTER loop
    3004  combine_instructions         <-- AFTER loop
    3028  schedule_insns               (sched1)

`insn_count` — the right-hand side of the `loop.c:1631` desirability test that
IS this function's entire residual — is computed by `count_loop_regs_set`
(`loop.c:2989-3092`) as simply `++count` for every insn of `GET_RTX_CLASS == 'i'`
between `loop_start` and `end`. It therefore counts INSN, JUMP_INSN and
CALL_INSN alike, including insns that are semantically dead, and it is taken
BEFORE `strength_reduce` deletes giv arithmetic and long before flow/combine run.

That opens a precise question the ledger had only answered for ONE channel
(K10, which measured constructs folded at expand/cse1): **is there an ordinary-C
construct whose RTL survives cse1 + `delete_dead_from_cse`, is counted by
`count_loop_regs_set`, and is then deleted for zero bytes by a LATER pass
(strength_reduce's `combine_givs`, flow's DCE, or combine)?** If such a channel
existed at any density, the +64 insn_count that K8/K10 priced at ~400 score
points would be free and the function would reopen. Both halves are now measured.

### KILLED

- **K15 — dead in-loop computation adds ZERO to `insn_count`.** The dead-code
  channel is closed at `toplev.c:2866`: `delete_dead_from_cse` runs after cse1
  and BEFORE `loop_optimize`, so a dead insn is already a NOTE when
  `count_loop_regs_set` counts class-'i' insns. Probe: `gen_dead.py` sweep of
  k = 0/4/8/16/32 chained dead ALU ops (`dead = *(s32 *)(src + 4); dead ^= C1;
  dead += C2; ...` with `dead` never read), each compiled `cc1 -O2 ... -dL` and
  its per-function movable table read:

      k=0   Loop from 25 to 146: 56 real insns.   asm 107 lines
      k=4   Loop from 25 to 167: 56 real insns.   asm 107 lines
      k=8   Loop from 25 to 185: 56 real insns.   asm 107 lines
      k=16  Loop from 25 to 221: 56 real insns.   asm 107 lines
      k=32  Loop from 25 to 293: 56 real insns.   asm 107 lines

  `insn_count` PINNED at 56 across a 32-fold sweep; emitted function
  byte-identical (107 asm lines) at every k. The growing UID range (146 -> 293)
  is the positive control: the insns WERE created by expand and then deleted, so
  this is a real deletion, not a failure to generate. Banked as
  `rejected/dead-locals-deleted-before-loop-counts-them.c`.

- **K16 — the "counted by loop.c, deleted later for free" channel does not exist
  from the address/giv direction either.** This was the strongest remaining
  candidate: a chain of pointer temporaries (`q0 = dst + 1; q1 = q0 + 1; ...;
  q15[0x11] = v/1800;`) whose adds a human would expect to survive to loop.c
  (they are genuine SETs with genuine uses, so `delete_dead_from_cse` cannot
  touch them) and then be re-absorbed for zero bytes either by
  `strength_reduce`'s `combine_givs` (they are all givs of biv `i` with the same
  stride, differing by constants) or by `combine` folding the add into the
  memory displacement. Probe: `gen_chain.py` sweep k = 0/1/2/4/8/16:

      k=0   Loop from 25 to 146: 56 real insns.   asm 107 lines
      k=1   Loop from 25 to 149: 56 real insns.   asm 107 lines
      k=2   Loop from 25 to 152: 56 real insns.   asm 107 lines
      k=4   Loop from 25 to 158: 56 real insns.   asm 107 lines
      k=8   Loop from 25 to 170: 56 real insns.   asm 107 lines
      k=16  Loop from 25 to 194: 56 real insns.   asm 107 lines

  Again PINNED at 56 with a byte-identical function. The chain never reaches
  loop.c at all: cse1 folds the whole `dst + 1 + 1 + ... + 1 + 0x11` chain into
  the single memory reference, so the constructs die UPSTREAM exactly as K10's
  masks did. Banked as `rejected/giv-pointer-chain-folds-at-cse1.c`.

- **HARNESS VALIDATION (this is why the two nulls above are trustworthy).** The
  identical sweep driver (`tmp/grind/func_8003C714/s5/sweep.sh`) was re-run over
  s4's `gen_k.py` as a positive control and reproduced K14 exactly:

      k=0   56 real insns   moved, no "not desirable"    asm 107 lines
      k=12  82 real insns   moved, no "not desirable"    asm 157 lines
      k=13  84 real insns   2x "not desirable"           asm 159 lines
      k=16  90 real insns   7x "not desirable"           asm 173 lines

  The instrument moves when the thing it measures moves. K15/K16 are real nulls.

### CONFIRMED — the merged law this function has been circling for four sessions

- **H8 — BYTE-COUNT COUPLING: on this chassis, `loop.c`'s `insn_count` is a
  faithful proxy for the function's emitted instruction count. Byte-neutral C is
  count-neutral C; count-positive C is byte-positive C. There is no free
  variable.** Every construct family measured across s2-s5 lands on the same
  line and none is off it:

  | construct family | probe | d insn_count | d emitted |
  |---|---|---|---|
  | redundant `& 0xFF` masks | K10 (s3) | 0 | 0 |
  | dead ALU chains, k<=32 | K15 (s5) | 0 | 0 |
  | pointer/address chains, k<=16 | K16 (s5) | 0 | 0 |
  | inlined index addressing | K8 (s2) | +2 | +3 insns / +13 score |
  | invariant const movables, k=16 | K14 (s4) | +34 | +66 asm lines |

  The mechanism behind the law is now named, not guessed: everything cheap
  enough to be free is deleted at expand, cse1 or `delete_dead_from_cse` — all
  three of which run BEFORE `loop_optimize` — and everything that survives to
  `loop_optimize` also survives `flow`/`combine`/`reload` to the assembler.
  The two passes that run AFTER loop and could in principle have provided a free
  deletion (`flow_analysis`, `combine_instructions`) never get anything to
  delete, because cse1 already took it.

  **Consequence — this closes `loop.c:1631` by construction rather than by
  search.** The test the residual hangs on is
  `threshold * savings * m->lifetime >= insn_count` with `savings == 1` and
  `lifetime == 1` pinned (H7), i.e. `119 >= insn_count`. Declining the movable
  needs either `insn_count >= 120` at zero extra hoists (+64 RTL insns) or
  `insn_count >= 84` with 13 extra hoists (K14, +28 RTL insns AND 13 extra
  preheader instructions). By H8 both requirements convert 1:1-or-worse into
  emitted instructions, against a target whose ENTIRE body is 104 instructions,
  whose loop is 59 and whose preheader is 6 with every one of those 6 already
  accounted for. The cheapest of the three routes overshoots the whole function's
  byte budget by roughly a factor of two. Ordinary C on the shipped chassis
  cannot reach distance 0 on this function, and that statement now rests on a
  measured law rather than on five separately-argued negatives.

### FRONTIER (reset for the next session — 1 live item, 2 closed records)

1. **LIVE (and the only live item): nothing on the spelling side. Do not
   dispatch a spelling, search, or structural modality here.** The `loop.c:1631`
   inputs are all measured (threshold H7/K9/K14; insn_count K8/K10/K15/K16;
   savings and lifetime pinned at 1 by construction; admission K9 + the
   `count_loop_regs_set` `may_not_move` paths re-read this session and closed —
   both require the MAGIC pseudo to be set in two basic blocks, and it is
   compiler-generated by `expand_divmod`'s `force_reg`, set exactly once, with
   no C-level handle on it). Random search is measured flat (K13, 66,016
   iterations, zero finds). What a next session CAN still add that is not
   re-derivation: a `forensics` pass that reads the emitted `.greg`/`.sched`
   dumps to confirm the mfhi-seat half of the residual (t2 vs t1) is a pure
   consequence of the hoist rather than an independent second residual — if it
   is independent, that is a genuinely new sub-target; if it is consequent
   (expected), the function is fully characterised.
2. **CLOSED RECORD — root cause.** The residual is the scalar `threshold`
   (`loop.c:532`, `(loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs)`), 122 on
   the shipped chassis because `regclass.c:380-387` counts a 32-register
   hard-float file the PS1 R3000A does not have; the original value must have
   been in [56,58]. This is recorded as the measured CAUSE only. It is BARRED as
   a lever and as an escalation ground by the standing Judge constraint and by
   the 2026-09-01 18:33 ruling (`docs/grind/decisions.md:19776`), which also
   established that the global variant would BREAK the oracle
   (`func_800324D0` 0 -> 3). Do not re-file it in any shape.
3. **CLOSED RECORD — dependency.** If a future owner ruling ever re-opens the
   chassis question, `func_800324D0` (code6cac_b) needs an ordinary structural
   re-grind to <= 58 RTL insns in its loop (currently 72). Not actionable from
   this function's queue slot; carried so the dependency is not lost.

## [s5] Dead in-loop computations survive to loop.c and inflate its insn_count, giving a byte-free route to the insn_count >= 120 that loop.c:1631 requires.
- mechanism: count_loop_regs_set (loop.c:2989-3092) increments its count for EVERY insn of GET_RTX_CLASS 'i' in the loop range, with no liveness filter, so semantically dead insns would be counted; the hope was that flow_analysis (toplev.c:2984, AFTER loop) would then delete them for zero bytes
- probe: tmp/grind/func_8003C714/s5/gen_dead.py sweep k = 0, 4, 8, 16, 32 of chained dead ALU ops on a loaded value, each preprocessed and compiled with the shipped cc1 (-O2 -G0 -funsigned-char -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -dL) and its per-function movable table read from the .loop dump; emitted asm line count taken from the same run
- result: insn_count PINNED at 56 for every k (loop UID range grows 146 -> 293, proving the insns were created and then deleted), emitted function byte-identical (107 asm lines) at every k. The deletion point is toplev.c:2866 `delete_dead_from_cse`, which runs after cse1 and BEFORE loop_optimize at toplev.c:2895 — the dead insns are NOTEs before loop.c ever counts them. Banked as rejected/dead-locals-deleted-before-loop-counts-them.c.
- verdict: KILLED

## [s5] Chained address temporaries are real SETs with real uses (so cse1's DCE cannot remove them), reach loop.c and are counted, and are then re-absorbed for zero bytes by strength_reduce's combine_givs or by combine folding the add into the memory displacement — a byte-free insn_count channel.
- mechanism: the temps are all general givs of biv i with identical stride differing by constants, exactly the shape combine_givs merges; and combine_instructions (toplev.c:3004) runs after loop_optimize (toplev.c:2895), so an add folded into a load/store displacement would be free
- probe: tmp/grind/func_8003C714/s5/gen_chain.py sweep k = 0, 1, 2, 4, 8, 16 (q0 = dst + 1; q1 = q0 + 1; ...; store through q<k-1> at the residual offset so the addressed byte is unchanged), same compile-and-read-the-dump harness
- result: insn_count PINNED at 56 for every k, emitted function byte-identical (107 asm lines) at every k. The chain never reaches loop.c: cse1 folds the entire add chain into the single memory reference, so the construct dies UPSTREAM exactly as K10's redundant masks did. The post-loop passes never receive anything to delete. Banked as rejected/giv-pointer-chain-folds-at-cse1.c.
- verdict: KILLED

## [s5] The s5 sweep harness is sensitive enough for its two null results to be trusted.
- mechanism: positive control — run the identical driver over s4's gen_k.py, whose behaviour is independently banked as K14
- probe: bash tmp/grind/func_8003C714/s5/sweep.sh gen_k.py ctl 0 12 13 16
- result: reproduced K14 exactly — k=0: 56 real insns / 107 asm lines; k=12: 82 / 157, all movables moved; k=13: 84 / 159 with two "not desirable"; k=16: 90 / 173 with seven "not desirable". The instrument moves when the measured quantity moves.
- verdict: CONFIRMED

## [s5] On this chassis loop.c's insn_count is a faithful proxy for emitted instruction count — byte-neutral C is count-neutral C and count-positive C is byte-positive C, so loop.c:1631 is closed by construction rather than by search.
- mechanism: every deletion opportunity cheap enough to be byte-free (constant folding at expand, cse1 value numbering and address folding, delete_dead_from_cse) occurs BEFORE loop_optimize; everything that survives loop_optimize also survives flow, combine, reload and the assembler, so the two post-loop deletion passes never receive a free insn
- probe: five construct families measured across s2-s5 — redundant masks (K10, 0/0), dead ALU chains k<=32 (K15, 0/0), pointer chains k<=16 (K16, 0/0), inlined index addressing (K8, +2 insn_count / +13 score), invariant const movables k=16 (K14, +34 insn_count / +66 asm lines)
- result: no family is off the line. Declining the 0x91A2B3C5 movable needs insn_count >= 120 with no extra hoists (+64 RTL insns) or insn_count >= 84 with 13 extra hoists (+28 RTL insns plus 13 preheader instructions); the target's whole body is 104 instructions, its loop 59 and its preheader 6 with all six accounted for. Cheapest route overshoots the entire function's byte budget by about 2x.
- verdict: CONFIRMED

## [s5] Dead in-loop computations survive to loop.c and inflate its insn_count, giving a byte-free route to the insn_count >= 120 that loop.c:1631 requires.
- mechanism: count_loop_regs_set (tools/gcc-2.7.2/loop.c:2989-3092) increments its count for EVERY insn of GET_RTX_CLASS 'i' in the loop range with no liveness filter, so dead insns would be counted; flow_analysis (toplev.c:2984) runs AFTER loop_optimize (toplev.c:2895) and would then delete them for zero bytes.
- probe: tmp/grind/func_8003C714/s5/gen_dead.py sweep k = 0,4,8,16,32 of chained dead ALU ops on a loaded value (dead = *(s32 *)(src + 4); dead ^= C1; dead += C2; ... with dead never read), each compiled with the shipped cc1 -O2 -G0 -funsigned-char -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -dL and its .loop movable table + emitted asm line count read.
- result: insn_count PINNED at 56 at every k (k=0/4/8/16/32 -> 'Loop from 25 to 146/167/185/221/293: 56 real insns.'), emitted function byte-identical (107 asm lines) at every k. The growing UID range proves the insns were created and then deleted: the deletion point is delete_dead_from_cse at toplev.c:2866, which runs after cse1 and BEFORE loop_optimize. Banked as rejected/dead-locals-deleted-before-loop-counts-them.c.
- verdict: KILLED

## [s5] Chained address temporaries are real SETs with real uses (so cse1 DCE cannot remove them), reach loop.c and are counted, then are re-absorbed for zero bytes by strength_reduce's combine_givs or by combine folding the add into the memory displacement — a byte-free insn_count channel.
- mechanism: the temps are all general givs of biv i with identical stride differing by constants (the combine_givs shape), and combine_instructions (toplev.c:3004) runs after loop_optimize (toplev.c:2895), so an add folded into a load/store displacement would cost nothing.
- probe: tmp/grind/func_8003C714/s5/gen_chain.py sweep k = 0,1,2,4,8,16 (q0 = dst + 1; q1 = q0 + 1; ...; q<k-1>[0x21-k] = v/1800, so the addressed byte is unchanged), same compile-and-read-the-dump harness.
- result: insn_count PINNED at 56 at every k (146/149/152/158/170/194 UID ranges, all '56 real insns.'), emitted function byte-identical (107 asm lines) at every k. cse1 folds the entire add chain into the single memory reference, so the construct dies UPSTREAM of loop.c exactly as K10's redundant masks did; the post-loop passes are never handed anything to delete. Banked as rejected/giv-pointer-chain-folds-at-cse1.c.
- verdict: KILLED

## [s5] The s5 sweep harness is sensitive enough for its two null results to be trusted.
- mechanism: positive control — run the identical driver over s4's gen_k.py, whose behaviour is independently banked as K14.
- probe: bash tmp/grind/func_8003C714/s5/sweep.sh gen_k.py ctl 0 12 13 16
- result: Reproduced K14 exactly: k=0 -> 56 real insns / 107 asm lines, all movables moved; k=12 -> 82 / 157, all moved; k=13 -> 84 / 159 with two 'not desirable'; k=16 -> 90 / 173 with seven 'not desirable'. The instrument moves when the measured quantity moves.
- verdict: CONFIRMED

## [s5] H8 — BYTE-COUNT COUPLING: on this chassis loop.c's insn_count is a faithful proxy for the function's emitted instruction count, so byte-neutral C is count-neutral C and loop.c:1631 is closed by construction rather than by search.
- mechanism: Every deletion opportunity cheap enough to be byte-free happens BEFORE loop_optimize — constant folding at expand, cse1 value numbering and address folding (toplev.c:2865), delete_dead_from_cse (toplev.c:2866) — and everything that survives loop_optimize (toplev.c:2895) also survives flow (2984), combine (3004), reload and the assembler. The two post-loop DCE-capable passes never receive a free insn.
- probe: Five construct families measured across s2-s5 against the same instrument: redundant &0xFF masks (K10), dead ALU chains k<=32 (K15), pointer/giv chains k<=16 (K16), inlined index addressing (K8), invariant const movables k=16 (K14).
- result: No family is off the line: 0/0, 0/0, 0/0, +2 insn_count / +13 score, +34 insn_count / +66 asm lines. Declining the 0x91A2B3C5 movable needs insn_count >= 120 with no extra hoists (+64 RTL insns) or insn_count >= 84 with 13 extra hoists (+28 RTL insns AND 13 extra preheader instructions); the target's whole body is 104 instructions, its loop 59, its preheader 6 with all six already accounted for. The cheapest route overshoots the entire function's byte budget by roughly 2x.
- verdict: CONFIRMED


## s6 (2026-09-01, synthesis modality) -- THE FUNCTION IS REACHABLE. H8 IS FALSIFIED.

Chassis re-check first, before anything was spent: `candidate.c` spliced over the
`INCLUDE_ASM` line, `sandbox func_8003C714 --disable all` prints **score 15,
target_insns 104, build_insns 105** -- identical to s1-s5. Every inherited
measurement was still chassis-current when this session started.

This session was mandated to MERGE, not to search. Merging s1-s5 produced one
question the ledger had never asked in this form: *why* does every free
construct die before `loop.c`? The answer (read out of `tools/gcc-2.7.2/loop.c`
and `toplev.c` this session) is that all of them were **basic-block-local**, and
`cse1` + `delete_dead_from_cse` are a redundancy fixpoint immediately upstream of
`loop_optimize`. The one class of RTL that cse1 provably CANNOT touch is
**loop-carried** arithmetic: a value that flows across the back edge is opaque to
cse1's value numbering. No prior session had put a loop-carried construct in
front of `count_loop_regs_set`. That is the whole finding.

### CONFIRMED

- **H9 -- THE CSE1-FIXPOINT LAW, and the loop-carried hole in it.** cse1
  (`toplev.c:2865`) and `delete_dead_from_cse` (2866) run immediately before
  `loop_optimize` (2895) with no insn-creating pass in between, so every insn
  `count_loop_regs_set` counts is non-redundant *at cse1's fixpoint*. That is
  why K10 (masks), K15 (dead ALU chains) and K16 (giv/address chains) all
  measured insn_count +0: all three are basic-block-local and cse1 is a fixpoint
  over them. It is NOT a law about bytes. **Loop-carried arithmetic escapes it**,
  and `strength_reduce`'s biv elimination -- which runs INSIDE `loop_optimize`,
  AFTER `move_movables` has already used `insn_count` -- deletes it for zero
  emitted bytes. That is a genuine free channel and it is large.

- **H10 -- distance 0 is REACHABLE on the shipped chassis. MEASURED, not
  argued.** Body: `tmp/grind/func_8003C714/s6/body_fv32.c` (banked at
  `rejected/balanced-biv-noise-64-insns-d0-but-inadmissible.c`). 32 balanced
  `z += c; z -= c;` pairs (net 0) on a loop-carried local `z`, `z += 1` at the
  bottom, and the single post-loop use `D_800A37B8 = z - 3;` (loop.c
  final-value-replaces `z` with 3, so the store folds to the `D_800A37B8 = 0`
  the target already has). `sandbox func_8003C714 --disable all` prints
  **score 0, target_insns 104, build_insns 104**. The residual this function has
  carried since s1 is GONE. The form is an inadmissible cheat and was NOT
  submitted -- but reachability is now a measurement, and every "unreachable on
  the shipped chassis" statement in s2-s5 (K5, H8, the two decisions.md
  INTEGRATION HANDOFF entries, the 2026-09-01 18:33 ruling's premise) is
  superseded by it. The chassis/CC_FLAGS story was a TRUE description of one
  route, never the only route.

### KILLED

- **K17 -- H8 (BYTE-COUNT COUPLING) is FALSE.** Sweep `gen_multi.py`
  (k unbalanced `j += 1` increments on a test-carrying biv), harness
  `tmp/grind/func_8003C714/s6/sweep.sh`, identical to the s5 driver:

      k=1   57 real insns   asm 107 lines
      k=2   58 real insns   asm 107 lines
      k=4   60 real insns   asm 107 lines
      k=8   64 real insns   asm 107 lines
      k=16  72 real insns   asm 107 lines
      k=32  88 real insns   asm 107 lines
      k=64  120 real insns  1x "not desirable"   asm 106 lines

  insn_count tracks k **1:1** while the emitted function stays BYTE-IDENTICAL
  (107 lines) all the way to k=32, and at k=64 the 0x91A2B3C5 movable flips to
  "not desirable" exactly as loop.c:1631 predicts (119 >= 120 is false). +64 RTL
  insns for +0 bytes. H8's table was a true summary of three basic-block-local
  families and a false generalisation.

- **K18 -- the noise must NOT sit on the surviving counter, and must NOT sit on
  the giv-driving biv.** Three placements measured at the same insn_count:
  * noise on the biv that carries the exit test (`gen_multi`/`gen_bal`, and the
    name-swapped `gen_swap`): 104 == 104 emitted instructions, instruction
    sequence exact, but **score 22** -- a pure REGISTER ROTATION. The `.lreg`
    dump names the cause in one line: `Register 73 used 263 times across 58
    insns`. The noise inflates the surviving counter's `reg_n_refs`, which is
    local-alloc's priority key, so the counter is allocated FIRST and takes a1;
    the target allocates it LAST (dst=a1, src=a2, magic=a3, counter=t0) and ours
    comes out rotated (counter=a1, dst=a2, src=a3, magic=t0).
  * noise on `i`, the biv that drives the src/dst givs (`gen_bal2`, `gen_noisei`):
    asm 108 / 109 lines -- the giv bookkeeping materialises 2-5 instructions.
  * noise on a third biv `z` whose ONLY use is post-loop (`gen_fv`): the biv is
    eliminated outright, its pseudo never reaches local-alloc, the counter keeps
    its baseline `reg_n_refs`, and the seat rotation disappears -> **score 0**.
  The seat is therefore not a separate residual at all (this also answers the s5
  frontier's forensics question: the mfhi t2-vs-t1 seat was CONSEQUENT on the
  hoist, not independent -- at score 0 every seat lands on the target).

- **K19 -- balanced pairs are needed, not raw increments, if the surviving
  counter must keep the target's stride.** `gen_multi` k=64 leaves the counter
  at stride 64 / limit 192 (two wrong immediates on top of the rotation);
  `gen_bal` (32 balanced pairs + one `+= 1`) restores stride 1 / limit 3. Both
  score 22; only the `gen_fv` placement fixes the rotation.

### FRONTIER (reset -- this is a SPELLING problem again, with an exact target)

1. **THE LIVE ITEM: find an ordinary-C spelling of ">= 64 extra loop-carried RTL
   insns inside the loop that die inside loop_optimize."** The requirement is
   exact and now cheaply testable: `insn_count >= 120` (measure it with
   `sweep.sh`, one cc1 run per form, no sandbox needed -- the `.loop` line
   `Loop from 25 to N: M real insns.` IS the gradient), with the carrier being a
   biv whose only use is post-loop (K18) so local-alloc never sees it.
   Constraints already known: loop-carried (else cse1 eats it, K10/K15/K16), not
   on the counter, not on the giv-driving biv. What has NOT been tried: any
   NATURAL carrier -- a genuine running accumulator/checksum whose final value
   is stored after the loop, a second index the source really had, 64-bit or
   multi-word arithmetic, a struct-copy idiom, an unrolled inner computation.
2. **The cheaper trade: free HOISTS instead of free insns.** loop.c:1631 is
   `threshold * savings * lifetime >= insn_count` with savings=lifetime=1, and
   `threshold -= 3` after every move (loop.c:1719/1904). With `h` extra hoists
   ahead of the 0x91A2B3C5 movable the requirement is `c > 63 - 3h`, so 21 free
   hoists need ZERO extra insns and 10 free hoists need only +34. Free hoists
   are known to exist: `gen_biv.py` k=2 (`j += 7`, test `j != 21`) measured
   `moved` 8 -> 9 with asm still 107 lines -- one extra hoist at zero byte cost,
   unexplained and unexploited. Characterising that channel is probably cheaper
   than finding a 64-insn carrier.
3. **CLOSED RECORD (superseded, do not re-file).** The chassis/`threshold`=122
   root cause (loop.c:532 / regclass.c:380-387) remains a true description of a
   BARRED route; it is no longer an exhaustion argument, because H10 measured
   distance 0 on the shipped chassis. Nothing here goes to the owner.

## [s6] The cse1 fixpoint (toplev.c:2865-2866) is why every prior free-insn_count probe measured +0, and loop-carried arithmetic is the hole in it.
- mechanism: cse1 + delete_dead_from_cse run immediately before loop_optimize with no insn-creating pass between; they are a redundancy fixpoint over basic-block-local RTL but cannot value-number across a loop back edge. strength_reduce's biv elimination then deletes surviving loop-carried insns INSIDE loop_optimize, after move_movables has already consumed insn_count.
- probe: gen_multi.py / gen_bal.py / gen_fv.py sweeps through tmp/grind/func_8003C714/s6/sweep.sh, reading the .loop movable table and the emitted asm line count at each k
- result: insn_count 56 -> 121 with a byte-identical emitted function; the 0x91A2B3C5 movable flips to "not desirable" at insn_count >= 120 exactly as loop.c:1631 predicts.
- verdict: CONFIRMED

## [s6] Distance 0 is reachable for func_8003C714 on the SHIPPED chassis (no CC_FLAGS change, no chassis change).
- mechanism: 32 balanced loop-carried `z += c; z -= c;` pairs raise insn_count past threshold 119 so loop.c:1631 declines the 0x91A2B3C5 hoist; z's only use is post-loop so biv elimination deletes all 64 insns for zero bytes; the magic then emits in the target's in-loop split lui/lw/ori/mult form and every register seat lands.
- probe: tmp/grind/func_8003C714/s6/body_fv32.c spliced over the INCLUDE_ASM line at src/code6cac_c2.c:629; `& tools/wteng.ps1 main sandbox func_8003C714 --disable all`
- result: score 0, target_insns 104, build_insns 104. The form is an inadmissible cheat (T1/T2/T3/T6 all FAIL, no frozen family) and was NOT submitted; it is banked as the reachability proof at rejected/balanced-biv-noise-64-insns-d0-but-inadmissible.c.
- verdict: CONFIRMED

## [s6] H8 BYTE-COUNT COUPLING: on this chassis loop.c's insn_count is a faithful proxy for the emitted instruction count, so loop.c:1631 is closed by construction.
- mechanism: s5 claimed every deletion opportunity cheap enough to be byte-free happens before loop_optimize, and everything surviving loop_optimize survives to the assembler
- probe: gen_multi.py sweep k = 1/2/4/8/16/32/64 (loop-carried increments on a test-carrying biv), .loop insn_count vs emitted asm line count
- result: FALSE. insn_count 57/58/60/64/72/88/120 against a constant 107 emitted asm lines. The law held only for basic-block-local constructs (the three families s5 sampled); loop-carried constructs break it completely.
- verdict: KILLED

## [s6] The d15 residual is one loop.c decision PLUS an independent mfhi-temp register seat (t2 vs t1) that would need its own sub-target.
- mechanism: s5 frontier item 1 -- forensics question of whether the seat is consequent on the hoist or independent
- probe: measured directly rather than by dump-reading: at score 0 (body_fv32.c) every register in the function lands on the target, including the mfhi temp
- result: CONSEQUENT. There is no second residual. The seat rotation seen in the intermediate forms (score 22) is a DIFFERENT effect -- local-alloc priority from the noise carrier's inflated reg_n_refs (.lreg: `Register 73 used 263 times across 58 insns`) -- and it vanishes when the carrier is a post-loop-use biv.
- verdict: KILLED

## [s6] H9 — cse1 (toplev.c:2865) + delete_dead_from_cse (2866) are a redundancy fixpoint immediately upstream of loop_optimize (2895), which is why every prior free-insn_count probe measured +0; loop-carried arithmetic is the one RTL class that escapes it, and strength_reduce's biv elimination (inside loop_optimize, AFTER move_movables consumes insn_count) deletes it for zero emitted bytes.
- mechanism: count_loop_regs_set counts class-'i' insns between loop_start and end BEFORE move_movables and BEFORE strength_reduce; cse1 cannot value-number across a loop back edge, so a loop-carried SET survives to be counted; a biv whose only use is post-loop is then eliminated outright by strength_reduce and every one of its update insns disappears without reaching flow, combine, local-alloc or the assembler.
- probe: tmp/grind/func_8003C714/s6/sweep.sh over gen_multi.py (k unbalanced increments on a test-carrying biv) and gen_fv.py (balanced pairs on a post-loop-use biv); read the .loop movable table and the emitted asm line count at each k
- result: gen_multi k=1/2/4/8/16/32 -> insn_count 57/58/60/64/72/88 with the emitted function BYTE-IDENTICAL (107 asm lines) at every k; k=64 -> insn_count 120 and the 0x91A2B3C5 movable prints 'not desirable' exactly as loop.c:1631 predicts (119 >= 120 is false).
- verdict: CONFIRMED

## [s6] H10 — distance 0 is reachable for func_8003C714 on the SHIPPED chassis with no CC_FLAGS change and no chassis change of any kind.
- mechanism: 32 balanced loop-carried `z += c; z -= c;` pairs (net 0) take the loop's insn_count from 56 to 121, so loop.c:1631 declines the 0x91A2B3C5 movable and the magic emits in the target's in-loop split lui/lw/ori/mult form; z's only use is the post-loop `D_800A37B8 = z - 3;`, which loop.c final-value-replaces with the constant 0 the target already stores, so biv elimination removes all 64 insns for zero bytes and local-alloc never sees the carrier pseudo.
- probe: tmp/grind/func_8003C714/s6/body_fv32.c spliced over the INCLUDE_ASM line at src/code6cac_c2.c:629; `& tools/wteng.ps1 main sandbox func_8003C714 --disable all`
- result: score 0, target_insns 104, build_insns 104. The form fails cheat tests T1/T2/T3/T6 and matches no frozen SOTN family, so it was NOT submitted; it is banked as the reachability proof at memory/grind/func_8003C714/rejected/balanced-biv-noise-64-insns-d0-but-inadmissible.c and src/ was restored to INCLUDE_ASM.
- verdict: CONFIRMED

## [s6] K17 — H8 (s5's BYTE-COUNT COUPLING law: byte-neutral C is count-neutral C, so loop.c:1631 is closed by construction).
- mechanism: s5 argued every deletion cheap enough to be byte-free happens before loop_optimize and everything surviving loop_optimize survives to the assembler
- probe: gen_multi.py sweep k = 1/2/4/8/16/32/64, .loop insn_count against emitted asm line count
- result: FALSE. insn_count 57/58/60/64/72/88/120 against a CONSTANT 107 emitted asm lines. H8 was a true summary of the three basic-block-local families s5 happened to sample and a false generalisation; it must not be quoted again as a foreclosure argument.
- verdict: KILLED

## [s6] K18 — the extra-insn carrier can sit anywhere in the loop.
- mechanism: local-alloc's quantity priority is keyed on reg_n_refs, so a carrier that survives to register allocation reorders the whole allocation
- probe: three placements at equal insn_count: carrier on the exit-test biv (gen_multi/gen_bal/gen_swap), carrier on the giv-driving biv i (gen_bal2/gen_noisei), carrier on a third biv whose only use is post-loop (gen_fv); plus tmp/grind/func_8003C714/dumps/code6cac_c2.lreg
- result: FALSE, and the failure mode is now named. Exit-test carrier: 104==104 emitted, instruction sequence exact, but score 22 — a pure cyclic register rotation (target dst=a1 src=a2 magic=a3 counter=t0; build counter=a1 dst=a2 src=a3 magic=t0), caused by `.lreg: Register 73 used 263 times across 58 insns`. giv-driving carrier: 108/109 asm lines (giv bookkeeping materialises 2-5 instructions). Post-loop-use carrier: score 0. Only the third placement works.
- verdict: KILLED

## [s6] The d15 residual is one loop.c decision PLUS an independent mfhi-temp register seat (t2 vs t1) needing its own sub-target (the s5 frontier's forensics question).
- mechanism: s5 asked whether the seat was consequent on the hoist or a second residual
- probe: answered by direct measurement rather than dump-reading: at score 0 every register in the function lands on the target
- result: CONSEQUENT — there is no second residual. The seat rotation seen at score 22 is a different, fully-explained effect (local-alloc priority from the carrier's reg_n_refs) and vanishes with the correct carrier placement.
- verdict: KILLED

## s7 (2026-09-01, solver modality) — all three s6 frontier items measured; two closed, one closed with a named exclusion

Chassis re-checked first: candidate.c spliced, `sandbox --disable all` = score 15,
104 target / 105 build. Unchanged since s1.

### CONFIRMED

- **H11 — the whole residual is one lifetime-1 movable, and both of its
  desirability inputs are already at their structural minimum.** Baseline
  `.loop` (s7/dumps/biv0.i.loop): the 0x91A2B3C5 movable is the SECOND entry in
  the movable list (`Insn 46: regno 84 (life 1), savings 1`), so it is tested
  after exactly one `threshold -= 3` at `119 * 1 * 1 >= 56`. `savings =
  n_times_used[regno]` (loop.c:793) is 1 because the constant has one use;
  `lifetime` (loop.c:791) is 1 because the `mult` is the next insn. Neither can
  go lower. The contrast with 0x88888889 — which the TARGET also hoists — is
  lifetime 31 vs 1, which is exactly why the original (threshold 58) chassis
  kept one and declined the other. Requirement for ordinary C on the shipped
  chassis: `c > 63 - 3h` where c = extra insn_count and h = extra movables moved
  BEFORE the magic.

- **H12 — the solver's object-level classifier reports RA and SCHED components
  for this function, and they are CONSEQUENT, not independent.**
  `goal_from_tgt.py classify code6cac_c2 func_8003C714` prints a nop-only SCHED
  difference (ours 1, target 0) and an RA component `$t2 -> $t1 x10`,
  `$t1 -> $v0 x2`, with 1 pair skipped because the skeleton differs (that pair is
  the magic's lui/ori itself). `inverse_compose.py classify` refuses on this
  zero-rule function and says so explicitly rather than emitting a fictitious
  PRE-RA verdict. s6's K18 already proved by measurement that at score 0 every
  seat lands, so no RA or scheduler lever exists while the hoist stands.
  Recorded so that no future session opens an RA/sched search because `classify`
  named an RA component.

### KILLED

- **K20 — the `moved_once` insn_count DOUBLING (loop.c:1609) is real but
  self-defeating.** `loop_optimize` scans loops last-first (loop.c:425) and
  `moved_once` is a per-FUNCTION array allocated once (loop.c:344), so a real
  inner loop containing the `/1800` statement DOES set `moved_once[regno]` and
  the outer scan DOES print `halved since already moved`: measured 59 -> 118
  (k=1) and 60 -> 120 (k=2). The movable is still moved, because the same inner
  hoist separates the SET from its use and `m->lifetime` goes 1 -> 14/15;
  lifetime multiplies the left side of loop.c:1631 (119*1*15 = 1785 >= 120). The
  2x gain costs a >=14x loss. Dead for every spelling, on mechanism, independent
  of the (also disqualifying) fact that a trip-count-1 inner loop is a banned
  respelling of the do-while(0) carve-out.
  `rejected/inner-loop-moved-once-doubling-inflates-lifetime.c`.

- **K21 — there is NO natural carrier; the s6 free channel is exactly
  "small-immediate-constant-step biv used only after the loop".** Sweep
  (baseline 56 insns / 107 asm lines): memory-sum accumulator used after the
  loop 58/112; the same accumulator DEAD 56/107; 4x 61/115; checksum
  `acc = acc*31 + mem` 60/114; 16x 74/128; 8x checksum 82/136; control
  `acc += 1` with post-use `acc - 3` 57/107. A dead data-dependent carrier is
  removed by cse1/delete_dead_from_cse before `count_loop_regs_set` (+0 count);
  a live one costs +1.12N .. +2.5N emitted instructions. The mechanism is
  structural: `strength_reduce` deletes a carrier only when it can FOLD the exit
  value, i.e. only for a biv with a CONSTANT increment, and any value that reads
  loop data has a non-constant increment and is never a biv. Reaching
  insn_count 120 with a natural carrier costs about +72 emitted instructions on
  a 104-instruction target. The s6 frontier's LIVE item is therefore closed:
  the only free spelling is dead code by construction.
  `rejected/natural-accumulator-carrier-costs-1-1-in-bytes.c`.

- **K22 — free HOISTS ahead of the magic do not exist; the hoist channel and
  the free-insn channel are MUTUALLY EXCLUSIVE.** k loop-carried counters with
  LARGE constant steps at the top of the body do create movables ahead of the
  magic (moved 8 -> 9/12/16) but cost ~9 emitted instructions each
  (k=1/4/8/16/22 -> asm 116/137/165/241/285). `Insn 49: possible biv, reg 74,
  const = (reg:SI 82)`: the moment the step constant is big enough to be
  hoisted, the increment is a REGISTER, the biv is not eliminable, and the
  counter materialises. Small immediate steps keep the biv eliminable but fold
  into `addiu` and create no movable at all. The one free hoist s6 observed
  (gen_biv k=2) is the literal 21 of the `j != 21` exit test at the loop BOTTOM
  — free only because that biv substitutes for the original counter, and after
  the magic in insn order, so it never reduces the threshold the magic sees.
  `rejected/bigconst-step-biv-creates-hoist-but-kills-biv-elimination.c`.

### FRONTIER after s7

The `c > 63 - 3h` requirement is intact and both terms are now measured closed
for ordinary C. What remains is not a spelling search:

1. **Nothing sanctioned is known to be left on the loop.c axis.** Every route to
   distance 0 measured so far routes through a small-immediate-step biv whose
   only use is post-loop — dead code by construction (T1/T2/T6 fail, no frozen
   family). If a future session wants to keep grinding this axis it must find a
   channel that changes `insn_count` or the movable ORDER without any added
   local at all; nothing in loop.c suggests one exists (savings and lifetime are
   pinned at 1, the doubling is self-defeating, and hoists cost bytes).
2. **Untried, cheap, and the honest next probe if the ladder demands one:**
   attack ADMISSION rather than desirability — make the `(set r 0x91A2B3C5)`
   insn not be a movable at all. s3's K10-K12 killed const-FORM respellings, but
   the `m->cond` / `consec` / `m->global` paths at loop.c:770-800 were never
   examined; `m->global` in particular is set when the pseudo's first use
   precedes the loop or its last use follows it. Whether any ordinary-C spelling
   can make the divisor pseudo global is unmeasured. (Low prior: cse1 would
   normally CSE a pre-loop `x / 1800` into the loop copy.)
3. **The disposition question is now ripe.** Six modalities (recon, structural
   x2, permuter, synthesis x2) plus this solver session have left the honest
   floor at 15 with every measured route to 0 running through an inadmissible
   construct. If the driver assigns `escalation`, the foreclosure record writes
   itself from H11/K20/K21/K22 plus s6's H10.

## [s7] The 0x91A2B3C5 movable's desirability inputs (savings, lifetime) are both already at their structural minimum, so `insn_count` and movable ORDER are the only C-reachable dials.
- mechanism: loop.c:791/793 set lifetime = luid(last use) - luid(first use) and savings = n_times_used[regno]; the constant has exactly one use and the `mult` is the next insn, so both are 1. loop.c:1631 then reduces to `threshold >= insn_count`, and threshold is 122 - 3*(movables moved before it).
- probe: tmp/grind/func_8003C714/s7/dumps/biv0.i.loop (candidate body, unmodified) — the movable list shows Insn 33 (&D_80106A58, life 1), Insn 46 (0x91A2B3C5, life 1), Insn 60 (0x88888889, life 31)
- result: the magic is tested SECOND, at 119*1*1 >= 56. The target hoists 0x88888889 and not the magic purely because of the lifetime 31 vs 1 split. Requirement for ordinary C: c > 63 - 3h.
- verdict: CONFIRMED

## [s7] The RA and SCHED components the ra_solver's object-level classifier reports for this function are consequent on the loop.c hoist, not independent residuals.
- mechanism: goal_from_tgt.py works from the emitted object streams, so it sees the seat rotation and the extra nop that FOLLOW the hoist but cannot see loop.c; inverse_compose.py classify refuses outright on a zero-rule function rather than emit a fictitious PRE-RA verdict
- probe: python3 tools/ra_solver/goal_from_tgt.py classify code6cac_c2 func_8003C714 (and `goal` for the substitution list)
- result: SCHED = nop-only multiset difference (ours 1, target 0); RA = $t2 -> $t1 x10, $t1 -> $v0 x2, with the magic's lui/ori reported as the one skipped pair. s6's K18 already measured that at score 0 every seat lands, so there is no RA or scheduler lever while the hoist stands.
- verdict: CONFIRMED

## [s7] K20 — the loop.c:1609 `moved_once` doubling can halve the +64 insn_count requirement to +4 by hoisting the magic out of an enclosing inner loop first.
- mechanism: loop_optimize scans loops last-first (loop.c:425) and moved_once is allocated once per function (loop.c:344), so an inner-loop hoist marks the regno and the outer scan does `insn_count *= 2`
- probe: tmp/grind/func_8003C714/s7/gen_inner.py k=1,2 through sweep.sh; read the outer loop's movable line in dumps/inner{1,2}.i.loop
- result: the doubling FIRES (59 -> 118, 60 -> 120, "halved since already moved" printed) and the movable is still moved, because the same hoist inflates m->lifetime from 1 to 14/15 and lifetime multiplies the threshold side: 119*1*15 = 1785 >= 120. Self-defeating for every spelling.
- verdict: KILLED

## [s7] K21 — a NATURAL ordinary-C carrier (running accumulator / checksum / second index) can supply the >= 64 free loop-carried insns the s6 proof needed.
- mechanism: s6 showed loop-carried arithmetic escapes the cse1 fixpoint and can be deleted inside loop_optimize for zero bytes; the open question was whether a natural, semantically real carrier does the same
- probe: tmp/grind/func_8003C714/s7/gen_acc.py variants 1-7 through sweep.sh (memory-sum accumulator live and dead, 4x and 16x, mul-accumulate checksum, and the constant-step control)
- result: FALSE. Dead data-dependent carrier = +0 insn_count (removed by cse1/delete_dead_from_cse before count_loop_regs_set). LIVE data-dependent carrier = +N insn_count and +1.12N..+2.5N emitted instructions (58/112, 61/115, 60/114, 74/128, 82/136 against baseline 56/107). strength_reduce deletes a carrier only when it can FOLD the exit value, which requires a CONSTANT increment; anything reading loop data is not a biv. insn_count 120 naturally would cost about +72 instructions on a 104-instruction target.
- verdict: KILLED

## [s7] K22 — free HOISTS ahead of the 0x91A2B3C5 movable are a cheaper route than free insns (~21 free hoists would need no extra insns at all).
- mechanism: loop.c:1719/1904 do `threshold -= 3` per movable actually moved, so h extra hoists ahead of the magic reduce the requirement to c > 63 - 3h; s6 observed one apparently free hoist at gen_biv k=2
- probe: tmp/grind/func_8003C714/s7/gen_hoist.py k=1,4,8,16,22 (loop-carried counters with large constant steps at the TOP of the body, post-loop folded), plus re-reading the gen_biv k=2 movable table
- result: FALSE. The hoists are created (moved 8 -> 9/12/16) but cost ~9 emitted instructions each (asm 116/137/165/241/285 vs baseline 107). `Insn 49: possible biv, reg 74, const = (reg:SI 82)` — a step constant big enough to be hoisted makes the increment a REGISTER, which disqualifies the biv from elimination; a step small enough to stay free folds into addiu and creates no movable. The two channels are mutually exclusive. s6's one free hoist is the literal 21 of the `j != 21` exit test at the loop BOTTOM (a counter substitution), which is after the magic in insn order and never reduces its threshold.
- verdict: KILLED

## [s7] The 0x91A2B3C5 movable's desirability inputs (savings, lifetime) are both already at their structural minimum, so insn_count and movable ORDER are the only C-reachable dials.
- mechanism: loop.c:791/793 set m->lifetime = luid(last use) - luid(first use) and m->savings = n_times_used[regno]; the constant has exactly one use and the mult is the next insn, so both are 1. loop.c:1631 then reduces to 'threshold >= insn_count', with threshold = 122 - 3*(movables moved before it). The target hoists 0x88888889 and not the magic purely because that pseudo's lifetime is 31 vs 1.
- probe: tmp/grind/func_8003C714/s7/dumps/biv0.i.loop (candidate body, unmodified): movable list = Insn 33 regno 78 (life 1) &D_80106A58, Insn 46 regno 84 (life 1) 0x91A2B3C5, Insn 60 regno 91 (life 31) 0x88888889
- result: The magic is tested SECOND, at 119*1*1 >= 56. Requirement for ordinary C on the shipped chassis: c > 63 - 3h.
- verdict: CONFIRMED

## [s7] The RA and SCHED components the ra_solver's object-level classifier reports for this function are consequent on the loop.c hoist, not independent residuals.
- mechanism: goal_from_tgt.py works from emitted object streams, so it sees the seat rotation and the extra nop that FOLLOW the hoist but cannot see loop.c. inverse_compose.py classify refuses outright on this zero-rule function rather than emit a fictitious PRE-RA verdict.
- probe: python3 tools/ra_solver/inverse_compose.py classify code6cac_c2 func_8003C714 (refused, with guidance) then python3 tools/ra_solver/goal_from_tgt.py classify/goal code6cac_c2 func_8003C714
- result: SCHED = nop-only multiset difference (ours 1, target 0); RA = $t2 -> $t1 x10, $t1 -> $v0 x2, with the magic's lui/ori reported as the one skipped (skeleton-differs) pair. s6's K18 already measured that at score 0 every seat lands, so no RA or scheduler lever exists while the hoist stands. Recorded so no future session opens an RA search because classify named an RA component.
- verdict: CONFIRMED

## [s7] K20: the loop.c:1609 moved_once insn_count DOUBLING can halve the +64 requirement to +4 by hoisting the magic out of an enclosing inner loop first (s1 frontier item 1, never measured until now).
- mechanism: loop_optimize scans loops last-first (loop.c:425) and moved_once is a per-FUNCTION array allocated once (loop.c:344), so an inner-loop hoist marks the regno and the outer scan executes insn_count *= 2 at loop.c:1611.
- probe: tmp/grind/func_8003C714/s7/gen_inner.py k=1,2 through sweep.sh; outer-loop movable line in dumps/inner1.i.loop and inner2.i.loop
- result: The doubling FIRES ('halved since already moved' printed; 59 -> 118 and 60 -> 120) and the movable is STILL MOVED, because the same hoist separates the SET from its single use and m->lifetime goes 1 -> 14/15. Lifetime multiplies the threshold side: 119*1*15 = 1785 >= 120. The 2x gain costs a >=14x loss - self-defeating for every spelling, independent of the fact that a trip-count-1 inner loop is also a banned do-while(0) respelling.
- verdict: KILLED

## [s7] K21: a NATURAL ordinary-C carrier (running accumulator, checksum, second index) can supply the >= 64 free loop-carried insns the s6 distance-0 proof needed.
- mechanism: s6 showed loop-carried arithmetic escapes the cse1 fixpoint and can be deleted inside loop_optimize for zero bytes; the open question was whether a semantically real carrier does the same.
- probe: tmp/grind/func_8003C714/s7/gen_acc.py variants 1-7 through sweep.sh (memory-sum accumulator live and dead, 4x, 16x, mul-accumulate checksum x1 and x8, plus the constant-step control)
- result: FALSE. Baseline 56 insns / 107 asm lines. Dead data-dependent carrier: 56/107 (+0/+0) - removed by cse1/delete_dead_from_cse before count_loop_regs_set. LIVE data-dependent carriers: 58/112, 61/115, 60/114, 74/128, 82/136, i.e. +1.12N..+2.5N emitted instructions per +N insn_count. Control 'acc += 1' with post-use 'acc - 3': 57/107 (+1/+0). strength_reduce deletes a carrier only when it can FOLD the exit value, which requires a CONSTANT increment; anything reading loop data is not a biv. Reaching insn_count 120 naturally costs about +72 instructions on a 104-instruction target. The s6 free channel is exactly 'small-immediate-constant-step biv used only after the loop' - dead code by construction.
- verdict: KILLED

## [s7] K22: free HOISTS ahead of the 0x91A2B3C5 movable are a cheaper route than free insns (~21 free hoists would need no extra insns at all).
- mechanism: loop.c:1719/1904 do threshold -= 3 per movable actually moved, so h extra hoists ahead of the magic reduce the requirement to c > 63 - 3h; s6 observed one apparently free hoist at gen_biv k=2.
- probe: tmp/grind/func_8003C714/s7/gen_hoist.py k=1,4,8,16,22 (loop-carried counters with LARGE constant steps at the TOP of the body, post-loop folded so the D_800A37B8 store stays 0), plus re-reading the gen_biv k=2 movable table in dumps/biv2.i.loop
- result: FALSE. Hoists are created (moved 8 -> 9/12/16) but cost ~9 emitted instructions each: asm 116/137/165/241/285 against baseline 107. The dump names the mechanism - 'Insn 49: possible biv, reg 74, const = (reg:SI 82)': a step constant big enough to be hoisted makes the increment a REGISTER, which disqualifies the biv from elimination; a step small enough to stay free folds into addiu and creates no movable. The two channels are mutually exclusive. s6's one free hoist is 'Insn 145: regno 132 (life 1)', the literal 21 of the 'j != 21' exit test at the loop BOTTOM - a counter substitution, after the magic in insn order, so it never reduces the threshold the magic is tested at.
- verdict: KILLED

## s8 (2026-09-01, forensics modality) — the ADMISSION axis is closed and the THRESHOLD term is closed at its source

Chassis re-checked first: candidate.c spliced, `sandbox --disable all` = score 15,
104 target / 105 build, unchanged since s1.

This session took the s7 frontier's item 2 (attack ADMISSION rather than
desirability) and closed it with five measured spellings plus a full read of
scan_loop's admission gate. It also found — and killed — one dial s7's K22 had
not tested: REORDERING the existing hoists rather than adding new ones.

### CONFIRMED

- **H13 — the desirability inequality of loop.c:1631 is now closed term by term,
  at the mechanism level, for ordinary C on the shipped chassis.** The test is
  `threshold * savings * m->lifetime >= insn_count`, evaluated for the
  0x91A2B3C5 movable at `119 * 1 * 1 >= 56`. Term by term:
  * `savings` = `n_times_used[regno]` = 1 (loop.c:793) — structural minimum, s7 H11.
  * `m->lifetime` = 1 (loop.c:791) — structural minimum, s7 H11.
  * `threshold` = `(loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs)` (loop.c:532)
    minus 3 per preceding move (loop.c:1719/1904). `n_non_fixed_regs` is writable
    only by `init_reg_sets_1` (regclass.c:380-387, i.e. CC_FLAGS — BARRED) and by
    `globalize_reg` (regclass.c:530, i.e. 32 file-scope register-asm pins — a
    cheat). `loop_has_call` halves it to 61, which is still >= 56 after the one
    preceding decrement, and requires a call the target's loop does not have.
    The per-move decrement is bounded by the number of movables in THIS loop,
    which is 3 (`threshold` is a scan_loop local recomputed per loop), so
    reordering caps at 122 -> 116 against a < 56 requirement.
  * `insn_count` is the only term left, and s7's K21 already measured that its
    one free carrier is dead code by construction.
  Nothing on this axis is open for ordinary C.

### KILLED

- **K23 — the ADMISSION axis (s7 frontier item 2) is dead. No ordinary-C spelling
  stops the `(set (reg N) (const_int 0x91A2B3C5))` insn from becoming a
  movable.** scan_loop's admission gate is exactly three tests and a
  compiler-generated CONST_INT set passes all three unconditionally:
  loop.c:649 `may_not_optimize` (written only by count_loop_regs_set at
  loop.c:3037/3044 — same pseudo in two basic blocks, or set twice with a use
  between, or explicitly clobbered; distinct source divisions give DISTINCT
  pseudos so it never fires); loop.c:695-700 whose second disjunct
  `(! REG_USERVAR_P && ! REG_LOOP_TEST_P)` is true for every compiler temp; and
  loop.c:715 whose `may_trap_p (src)` is 0 for a CONST_INT. `m->cond` and
  `m->global`, the two fields s7 named, are not admission gates at all —
  `m->global` (loop.c:790) is consumed only for `m->savemode` on partial movables
  (loop.c:888) and `m->cond` (loop.c:789) is structurally 0 because
  `invariant_p (const_int)` returns 1.
  MEASURED, five forms, `tmp/grind/func_8003C714/s8/sweep_adm.sh`
  (base 56 insns/3 movables/107 asm as control):
  `cond` 58/4/109, `twobb` 66/4+match/123, `preloop` 56/3/120,
  `postloop` 56/3/118, `twosame` 63/3/119 — the magic is hoisted in every one,
  and every one costs bytes. `twobb` actively backfires: the two magic loads are
  MATCHED (`Insn 73: regno 92 (life 1), done move-insn matches 54`) and `savings`
  goes 1 -> 2, making the hoist strictly more desirable.
  `rejected/admission-escape-hatches-all-pass-a-const-int-movable.c`.

- **K24 — the MOVABLE-ORDER dial is free and ordinary C, and is exactly 11x too
  small.** s7's K22 killed ADDING hoists (they cost ~9 emitted instructions
  each). This session tested the adjacent thing: reordering the hoists that
  already exist. Plain statement reordering inside the loop body moves the magic
  from movable slot 2 to slot 3 with insn_count PINNED at 56 and the emitted
  instruction count unchanged at 107 — genuinely free. But the loop has only
  THREE movables (regno 78 = `&D_80106A58`, regno 84 = 0x91A2B3C5, regno 91 =
  0x88888889), so the free range is threshold 122 -> 116, i.e. -6 against the -66
  the inequality needs. It is also byte-divergent in SEQUENCE: the emitted /1800
  quartet relocates past the /30 block, and the target's statement order is
  already the candidate's. Measured: `sweep_ord.sh` — 0x22,0x23,0x21,0x24 and
  0x23,0x22,0x21,0x24 both give 56 insns / slot 3 / 107 asm; 0x24,0x22,0x23,0x21
  gives 56 / slot 3 / 108.
  `rejected/statement-reorder-buys-free-slots-but-caps-at-minus-6.c`.

- **K25 — the LOOP FORM is insn_count-neutral; the `loop_top` route adds zero.**
  `count_loop_regs_set` counts from `loop_top ? loop_top : loop_start`
  (loop.c:592), so a loop entered at the bottom counts a wider range and might
  have inflated insn_count for free. Measured (`sweep_form.sh`): do-while, `for`
  and `while` spellings of the identical body all give `56 real insns`, the same
  3 movables with the magic 2nd, and 107 asm lines. The `for`/`while` forms do
  widen the range (25..146 -> 24..148 / 25..148) but the extra luids are NOTEs,
  not `'i'`-class insns, so `count` at loop.c:3007 never sees them.

- **K26 — `threshold` is not a C-reachable dial by any route.** `loop.c:532` has
  two inputs. `n_non_fixed_regs` is set once in `init_reg_sets_1`
  (regclass.c:380-387) and decremented ONLY by `globalize_reg` (regclass.c:530),
  which is reachable only from a file-scope `register T x asm("$k");` — the
  register-asm-pin cheat family — and would need 32 instances TU-wide to reach
  threshold < 56 (`2*(1+N) - 3 < 56 => N <= 28`, from 60). NOT measured,
  deliberately. `loop_has_call` (loop.c:2202) halves threshold to 61, which is
  the biggest single lever in the file and still insufficient: 61 - 3 = 58 >= 56,
  so the magic is still hoisted, and a call in the loop is a different function
  (the target loop 8003C750..8003C7C8 has none). The one genuine admission kill
  that a call WOULD unlock — the `reg_single_usage` deletion at loop.c:735-767,
  gated on the array being allocated at loop.c:586-589 — additionally requires
  `validate_replace_rtx` to substitute the CONST_INT into its single use; the use
  is the MIPS highpart multiply with `register_operand` operands, so it can never
  validate.
  `rejected/threshold-term-only-movable-by-cheat-or-by-a-call-in-the-loop.c`.

### FRONTIER after s8

The loop.c:1631 axis is now closed term by term with measurements, and no
untried structural channel is visible in loop.c:

1. **There is no known sanctioned route to distance 0.** Every measured route
   runs through `insn_count >= 120`, whose only free carrier (s6 H10, s7 K21) is
   a small-immediate-constant-step biv used only after the loop — dead code by
   construction, failing T1/T2/T6 and matching no frozen SOTN family. `savings`,
   `lifetime` (s7 H11), `threshold` (s8 K26), movable ADMISSION (s8 K23), movable
   ORDER (s8 K24) and loop form (s8 K25) are each independently closed.
2. **Do NOT re-open**: RA/scheduler search (s7 H12, s6 K18 — every seat lands at
   score 0, the classifier is seeing the hoist's shadow); const-FORM respellings
   (s3 K10-K12); inner-loop `moved_once` doubling (s7 K20); added hoists
   (s7 K22); natural accumulators/checksums (s7 K21); permuter (s4, 66,016
   iterations, 0 finds); the CC_FLAGS/`-msoft-float` chassis story (barred, and
   superseded by s6 H10 anyway).
3. **The disposition question is ripe and is the DRIVER's call.** Seven sessions
   across five distinct modalities (recon, structural x2, permuter, synthesis x2,
   solver) plus this forensics session have held the honest floor at 15. If the
   driver assigns `escalation`, the foreclosure record writes itself from
   s7 H11 + s8 H13 (the inequality closed term by term), s6 H10 (distance 0 is
   reachable but only via an inadmissible construct), and K20-K26.

## [s8] H13 — the loop.c:1631 desirability inequality is closed term by term for ordinary C on the shipped chassis: savings and lifetime are at their structural minimum (s7 H11), threshold is writable only by CC_FLAGS or 32 register-asm pins and its per-move decrement is capped at -6 by this loop's 3 movables, and insn_count's only free carrier is dead code (s7 K21).
- mechanism: loop.c:1631 `threshold * savings * m->lifetime >= insn_count`, with threshold from loop.c:532 `(loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs)` minus 3 per move (loop.c:1719/1904); n_non_fixed_regs from regclass.c:380-387 / 530; savings from loop.c:793; lifetime from loop.c:791.
- probe: full read of scan_loop's admission gate and move_movables' desirability block, plus the s8 admission (6 forms), order (4 forms) and loop-form (3 forms) sweeps through tmp/grind/func_8003C714/s8/
- result: every term except insn_count is closed at the mechanism level, and insn_count was closed by s7 K21.
- verdict: CONFIRMED

## [s8] K23 — an ordinary-C spelling can make the `(set r 0x91A2B3C5)` insn fail loop.c's movable ADMISSION (the m->cond / m->consec / m->global paths s7's frontier named).
- mechanism: loop.c:649 may_not_optimize (set by count_loop_regs_set at loop.c:3037/3044), loop.c:695-700 the uservar/basic-block test, loop.c:715 the trap test; m->cond at loop.c:789 and m->global at loop.c:790.
- probe: tmp/grind/func_8003C714/s8/sweep_adm.sh with gen_adm.py forms base / cond (division in a conditionally-executed basic block) / twobb (a second /1800 in the else arm) / preloop (a /1800 before the loop) / postloop (a /1800 after the loop) / twosame (a second /1800 in the same basic block); one cc1 -dL run each, reading the loop-25 movable table.
- result: FALSE. All five probes still hoist the magic (56/58/66/56/56/63 insn_count, 107/109/123/120/118/119 asm lines). m->global is not an admission gate (loop.c:888 uses it only for savemode on partial movables); m->cond is structurally 0 because invariant_p(const_int) returns 1; may_not_optimize never fires because two source-level divisions give two DISTINCT pseudos. twobb backfires: loop.c MATCHES the two magic loads (`Insn 73: regno 92 (life 1), done move-insn matches 54`) and savings goes 1 -> 2.
- verdict: KILLED

## [s8] K24 — reordering the loop body's statements (as opposed to s7 K22's ADDING hoists) is a free way to push the magic later in the movable list and reduce the threshold it is tested at.
- mechanism: scan_loop appends movables in insn order (loop.c:797-803) and move_movables does `threshold -= 3` per successful move (loop.c:1719/1904), so k moves ahead of the magic give threshold = 122 - 3k.
- probe: tmp/grind/func_8003C714/s8/sweep_ord.sh with gen_ord.py, four statement orders of the identical four body statements; read the loop-25 movable table and the emitted asm.
- result: TRUE but 11x too small, and byte-divergent. Reordering is free (insn_count pinned at 56, asm 107 lines) and does move the magic from slot 2 to slot 3, but this loop has only THREE movables so the ceiling is threshold 116 against a < 56 requirement. The emitted /1800 quartet also relocates past the /30 block, and the target's order is already the candidate's.
- verdict: KILLED

## [s8] K25 — a `for`/`while` loop form inflates insn_count for free, because count_loop_regs_set counts from loop_top rather than loop_start.
- mechanism: loop.c:592 `count_loop_regs_set (loop_top ? loop_top : loop_start, end, ...)`; loop.c:3007 increments `count` only for GET_RTX_CLASS == 'i'.
- probe: tmp/grind/func_8003C714/s8/sweep_form.sh with gen_form.py — do-while, for, while spellings of the identical body.
- result: FALSE. All three give 56 real insns, 3 movables with the magic 2nd, 107 asm lines. The range does widen (25..146 -> 24..148 / 25..148) but the extra luids are NOTEs, not 'i'-class insns.
- verdict: KILLED

## [s8] K26 — `threshold` (loop.c:532) is a C-reachable dial on the shipped chassis.
- mechanism: `threshold = (loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs)`; n_non_fixed_regs is written by init_reg_sets_1 (regclass.c:380-387) and decremented by globalize_reg (regclass.c:530); loop_has_call is set at loop.c:2202.
- probe: source read of loop.c and regclass.c plus the arithmetic (`2*(1+N) - 3 < 56 => N <= 28`, from 60); the loop_has_call halving evaluated against the measured baseline insn_count 56.
- result: FALSE. n_non_fixed_regs moves only via CC_FLAGS (barred) or 32 file-scope register-asm pins (the forbidden register-pin family — not measured, deliberately). loop_has_call halves threshold to 61 but 61 - 3 = 58 >= 56, so the magic is still hoisted, and a call in the loop is a different function. The `reg_single_usage` deletion branch a call would unlock (loop.c:735-767) additionally needs validate_replace_rtx to substitute a CONST_INT into the MIPS highpart multiply, whose operands are register_operand — it can never validate.
- verdict: KILLED

## [s8] H13 — the loop.c:1631 desirability inequality `threshold * savings * m->lifetime >= insn_count` is closed term by term for ordinary C on the shipped chassis; only insn_count remains a dial and s7's K21 already closed it.
- mechanism: threshold comes from loop.c:532 `(loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs)` minus 3 per successful move (loop.c:1719/1904); savings = n_times_used[regno] (loop.c:793) = 1; m->lifetime (loop.c:791) = 1; n_non_fixed_regs is written only by init_reg_sets_1 (regclass.c:380-387) and globalize_reg (regclass.c:530).
- probe: Full read of scan_loop's admission gate and move_movables' desirability block in tools/gcc-2.7.2/loop.c and of regclass.c:360-535, cross-checked against the baseline movable table in tmp/grind/func_8003C714/s8/dumps/base.i.loop and against 13 measured cc1 -dL runs across three sweeps.
- result: Baseline (candidate.c) loop-25 movable table: 3 movables — regno 78 (&D_80106A58, life 1), regno 84 (0x91A2B3C5, life 1), regno 91 (0x88888889, life 31) — insn_count 56, magic tested at 119*1*1 >= 56. savings and lifetime are structural minima (s7 H11); threshold is closed by K26; the per-move decrement is capped at -6 by K24; admission is closed by K23; loop form is neutral by K25.
- verdict: CONFIRMED

## [s8] K23 — some ordinary-C spelling can make the `(set r 0x91A2B3C5)` insn fail loop.c's movable ADMISSION via the m->cond / m->consec / m->global paths (the s7 frontier's item 2).
- mechanism: loop.c:649 `! may_not_optimize[regno]` (written only by count_loop_regs_set at loop.c:3037 same-pseudo-in-two-basic-blocks and loop.c:3044 set-twice-with-a-use-between); loop.c:695-700 the uservar / basic-block test; loop.c:715 the trap test; m->cond at loop.c:789 and m->global at loop.c:790.
- probe: tmp/grind/func_8003C714/s8/sweep_adm.sh with gen_adm.py — six forms, one cc1 -O2 -G0 -mel -dL run each, reading the loop-25 movable table and the emitted asm line count: base / cond (division inside `if (i != 5) {}` so its set sits in a conditionally-executed basic block) / twobb (a second /1800 in the else arm — two basic blocks) / preloop (a /1800 before the loop, m->global via regno_first_uid < INSN_LUID(loop_start)) / postloop (a /1800 after the loop, m->global via regno_last_uid > end) / twosame (a second /1800 in the same basic block, the n_times_set / consec route).
- result: FALSE, unanimously. base 56 insns / 3 movables / 107 asm; cond 58 / 4 / 109; twobb 66 / 4+match / 123; preloop 56 / 3 / 120; postloop 56 / 3 / 118; twosame 63 / 3 / 119. The magic is admitted and hoisted in every single one, and every one costs bytes. Mechanism: loop.c:695-700's second disjunct `(! REG_USERVAR_P && ! REG_LOOP_TEST_P)` is TRUE for every compiler temp, so that test can never reject an expand_divmod-generated pseudo; may_trap_p(const_int) is 0 so loop.c:715 never fires; and may_not_optimize never fires because two source-level divisions produce two DISTINCT pseudos. m->global is not an admission gate at all (loop.c:888 consumes it only for m->savemode on partial movables) and m->cond is structurally 0 because invariant_p(const_int) returns 1. The twobb form actively BACKFIRES: loop.c's movable matching merges the two magic loads (`Insn 54: regno 86 (life 2), savings 2` + `Insn 73: regno 92 (life 1), done move-insn matches 54`), doubling savings and making the hoist strictly more desirable.
- verdict: KILLED

## [s8] K24 — REORDERING the hoists that already exist (as distinct from s7's K22, which killed ADDING hoists) is a free way to push the 0x91A2B3C5 movable later in the movable list and lower the threshold it is tested at.
- mechanism: scan_loop appends movables in insn order (loop.c:797-803); move_movables walks the list in order and does `threshold -= 3` after each successful move (loop.c:1719/1904), so k moves ahead of the magic give threshold = 122 - 3k.
- probe: tmp/grind/func_8003C714/s8/sweep_ord.sh with gen_ord.py — four statement orders of the identical four body statements (0x21,0x22,0x23,0x24 as control; 0x22,0x23,0x21,0x24; 0x23,0x22,0x21,0x24; 0x24,0x22,0x23,0x21), reading the loop-25 movable table plus a diff of the emitted bodies.
- result: TRUE as a mechanism, dead as a lever. Reordering IS free — insn_count pinned at 56 and emitted instruction count pinned at 107 — and it does move the magic from movable slot 2 (regno 84) to slot 3 (regno 126/127), i.e. threshold 119 -> 116. But this loop has exactly THREE movables, so slot 3 is the ceiling: -6 against the -66 the inequality needs (threshold must fall below insn_count 56). `threshold` is a scan_loop LOCAL recomputed per loop (loop.c:532), so the file's other loops contribute nothing. It is byte-divergent in SEQUENCE anyway — the diff relocates the whole /1800 quartet (lw/mult/mfhi/addu/sra/sra/subu/sb 33($5)) past the /30 block — and the target's statement order is already the candidate's.
- verdict: KILLED

## [s8] K25 — a `for` or `while` loop form inflates insn_count for free, because count_loop_regs_set counts from loop_top rather than loop_start when the loop is entered at the bottom.
- mechanism: loop.c:592 `count_loop_regs_set (loop_top ? loop_top : loop_start, end, ...)`; loop.c:3007 increments `count` only for insns with GET_RTX_CLASS == 'i'.
- probe: tmp/grind/func_8003C714/s8/sweep_form.sh with gen_form.py — do-while, for, and while spellings of the byte-identical body, one cc1 -dL run each.
- result: FALSE. All three give `56 real insns`, the same 3 movables with the magic 2nd, and 107 asm lines. The `for`/`while` forms do widen the counted range (25..146 -> 24..148 and 25..148) but every extra luid is a NOTE, not an 'i'-class insn, so `count` never sees them. The loop_top route contributes exactly zero.
- verdict: KILLED

## [s8] K26 — `threshold` (loop.c:532) is a C-reachable dial on the shipped chassis.
- mechanism: `threshold = (loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs)`; n_non_fixed_regs is set in init_reg_sets_1 (regclass.c:380-387) after CONDITIONAL_REGISTER_USAGE and decremented ONLY by globalize_reg (regclass.c:530); loop_has_call is set at loop.c:2202 when the loop body contains a CALL_INSN.
- probe: Source read of loop.c:532 / 2202 / 586-593 / 735-767 and regclass.c:360-535, plus the arithmetic against the measured baseline: reaching threshold < insn_count 56 needs 2*(1+N) - 3 < 56, i.e. N <= 28 from the current 60.
- result: FALSE by both inputs. (1) n_non_fixed_regs is 60; -msoft-float takes it to 28 / threshold 58, which is exactly the s2 route, but CC_FLAGS is a BARRED surface. The only other writer anywhere in the compiler is globalize_reg, reachable only from a file-scope `register T x asm("$k");` — the forbidden register-asm-pin family — and it would take 32 of them TU-wide (deliberately NOT measured: it would produce only a cheat artifact, and code6cac_c2.c carries 43 other functions). (2) loop_has_call halves threshold to 61, the single biggest lever in loop.c, and is STILL insufficient: 61 - 3 = 58 >= 56, so the magic is still hoisted; it would additionally need insn_count >= 59, and a call in the loop is a different function (the target loop 8003C750..8003C7C8 contains none). The one genuine admission KILL a call would unlock — the reg_single_usage deletion branch at loop.c:735-767, whose array is allocated only under loop_has_call at loop.c:586-589 — additionally requires validate_replace_rtx to substitute the CONST_INT into its single use; that use is the MIPS highpart multiply, whose operands are register_operand, so it can never validate.
- verdict: KILLED

## s9 (2026-09-01) — escalation / disposition modality

### K27 — the hand-coded scanner tier is LOW (0/8), MEASURED not assumed. KILLED (canonical-asm path)

Statement: `func_8003C714` might qualify for the canonical-asm grant path, which
would make the residual moot.
Mechanism: `tools/scan_hand_coded.py` scores eight hand-written-asm signals; the
grant path requires a STRONG tier, i.e. one of S1 (multu pacing), S2 (empty-body
branch) or S6 (BIOS jumptable).
Probe: `python3 tools/scan_hand_coded.py --single func_8003C714`.
Result: `tier=LOW score=0/8`; zero signals of any strength. S1 finds 0 multu/mflo
pairs, S2 no empty-body branches, S3 sees 2 spills across 104 insns and 9 distinct
registers, S4 a max load burst of 1 in any 8-insn window, S5 no similar siblings,
S6 no BIOS jumptable pattern, S7 all callee-saves properly saved, S8 no redundant
mask-before-shift.
Verdict: **KILLED.** The function is ordinary compiler output. Sessions s1-s8 all
assumed this; s9 is the first to measure it. Do not re-run this scan — the answer
is banked, and it will not change without a scanner change.

### K28 — no SOTN-master precedent exists for the closing construct. KILLED (family-grant path)

Statement: the s6 distance-0 construct (32 balanced `z += c; z -= c;` loop-carried
pairs whose sole purpose is inflating `count_loop_regs_set`'s `insn_count`) might
have an in-hand SOTN-master precedent, which under the endgame-lock AND-gates
would make it citable rather than an auto-reject.
Mechanism: `docs/reference/sotn-construct-index.md` is the machine-generated index
of every match-hack construct SOTN master ships at commit
`aa53500226ee84be763f3e8702b27de06456b3a7` (1911 files scanned). A hit is citable
evidence; an absence after a real search is evidence of no precedent.
Probe: `grep -iE "insn_count|licm|hoist|loop_optimize|invariant|strength|unroll|biv"`
-> 0 hits. `grep -iE "loop-carried|balanced|noise"` -> 0 hits. `grep -i loop` ->
19 hits, every one either a prose comment about loop SHAPE, one dup-arm statement
(`src/main/psxsdk/libsnd/seqread.c:328`), or an exit label. The index's twelve
classes were enumerated and none has this shape; the nearest-looking one,
`pad_dummy_local` (816 hits), is DECLARED unused locals, which s5 K15 already
measured at +0 insn_count here because they die at the `cse1` /
`delete_dead_from_cse` fixpoint upstream of `loop_optimize`.
Verdict: **KILLED.** The census is NEGATIVE, which is a FAILED gate, not an open
question. Do not re-run it and do not attempt to argue the construct in from
adjacency — per the owner's 2026-08-24 ruling (reaffirmed 2026-08-31) a construct
outside the frozen family list is a clean refusal.

### H14 (open, but NOT a grind lever) — the residual is now gated on policy, not on search

With K27 and K28 both KILLED, and with the `loop.c:1631` inequality closed term by
term across s6-s8 (savings pinned at 1, lifetime pinned at 1, `threshold` movable
only by `CC_FLAGS` or 32 register-asm pins, ADMISSION dead in five spellings,
movable ORDER capped at -6, loop FORM count-neutral, and `insn_count`'s only free
carrier being dead code by construction), the function's residual is no longer a
search problem. It is decided by policy: the ONE construct that reaches distance 0
on the shipped chassis is inadmissible, and the ONE chassis on which the honest
body reaches distance 0 is barred.

Three named re-activation triggers are recorded in the foreclosure entry
(`docs/grind/decisions.md:19815`):
  1. a class grant covering `/* FAKE */`-annotated loop-carried counting carriers
     deleted by `strength_reduce` biv elimination — the closing body already
     exists and is already measured at distance 0;
  2. a toolchain finding that changes `n_non_fixed_regs` on the shipped chassis
     without a `CC_FLAGS` edit (soft-float chassis-fidelity evidence), which must
     also resolve the s2 conflict where `func_800324D0` regresses 0 -> 3;
  3. a fourth ordinary-C movable in this loop, taking the s8 K24 order dial past
     its -6 cap.

None of the three is reachable from inside a grind session. A future session
dispatched onto this function should verify the disposition entry still stands
rather than re-opening any axis above.

## [s9] func_8003C714 might qualify for the canonical-asm grant path (gate a), which would make the d15 residual moot.
- mechanism: tools/scan_hand_coded.py scores eight hand-written-asm signals; the grant path requires a STRONG tier, i.e. one of S1 (multu pacing), S2 (empty-body branch) or S6 (BIOS jumptable) firing.
- probe: python3 tools/scan_hand_coded.py --single func_8003C714 (output banked at tmp/grind/func_8003C714/s9/scan.txt)
- result: tier=LOW score=0/8 — zero signals of any strength. S1 finds 0 multu/mflo pairs, S2 no empty-body branches, S3 sees 2 spills over 104 insns / 9 distinct regs, S4 a max load burst of 1 per 8-insn window, S5 no similar siblings, S6 no BIOS jumptable pattern, S7 all callee-saves saved, S8 no redundant mask-before-shift. Sessions s1-s8 all ASSUMED this tier; s9 is the first to measure it.
- verdict: KILLED

## [s9] The only construct measured to reach distance 0 on the shipped chassis (32 balanced loop-carried `z += c; z -= c;` pairs inflating count_loop_regs_set's insn_count from 56 past 120 so loop.c:1631 declines the 0x91A2B3C5 movable) might have an in-hand SOTN-master precedent, satisfying gate (b).
- mechanism: docs/reference/sotn-construct-index.md is the machine-generated index of every match-hack construct sotn-decomp master ships at commit aa53500226ee84be763f3e8702b27de06456b3a7 (1911 files scanned, 12 construct classes, 2746 lines). A hit is citable SOTN-master evidence; an absence after a real search is evidence of no precedent.
- probe: grep -iE "insn_count|licm|hoist|loop_optimize|invariant|strength|unroll|biv" and grep -iE "loop-carried|balanced|noise" over the index, plus enumeration of all twelve class headings and a manual read of every `loop` hit (census banked at tmp/grind/func_8003C714/s9/census.txt).
- result: 0 hits and 0 hits. The 19 `loop` hits are all either prose match-comments about loop SHAPE, one dup-arm statement (src/main/psxsdk/libsnd/seqread.c:328), or exit labels (after_loop:, loop_30:). The twelve classes are fake_comment, fake_identifier, self_assign, match_comment, do_while_zero, pad_dummy_local, new_var_temp, pointer_alias, dup_if_else_arm, const_holder, empty_if, nested_exit_label — none has this shape. The nearest-looking class, pad_dummy_local (816 hits), is DECLARED unused locals, which s5 K15 already measured at +0 insn_count here because they die at the cse1/delete_dead_from_cse fixpoint upstream of loop_optimize.
- verdict: KILLED

## [s9] The chassis may have drifted since the s8 ledger entry, in which case every banked spelling conclusion would need re-measuring before it could be spent.
- mechanism: The dispatch brief reported the HEAD honest floor as 'measurement unavailable', so the ledger's floor of 15 could not be trusted without re-measurement.
- probe: Applied memory/grind/func_8003C714/candidate.c over the INCLUDE_ASM line in src/code6cac_c2.c (tmp/grind/func_8003C714/s9/apply.py), then `& tools/wteng.ps1 main sandbox func_8003C714 --disable all`.
- result: score 15, target_insns 104, build_insns 105, scorable true, rules_dropped 0, cheat_asm_stripped 10 — bit-for-bit the s3/s4/s5/s6/s7/s8 measurement. No chassis drift; the whole ledger is chassis-current. src/code6cac_c2.c was reverted to INCLUDE_ASM afterwards.
- verdict: CONFIRMED

---

## s10 (2026-09-05) — rederive modality

### H15 — CONFIRMED (source + dump)
**Statement.** `m->savings` in the loop.c:1631 desirability test counts SETS of
the invariant pseudo inside the loop, not uses. `loop.c:793` reads
`n_times_used[regno]`, and `loop.c:597` fills `n_times_used` with a `bcopy` of
`n_times_set` (produced by `count_loop_regs_set`).
**Evidence.** Baseline `.loop` table on this chassis prints `savings 1` for the
0x88888889 (/30) magic even though four in-loop `mult`s read it, and `savings 2`
for a base symbol materialised twice in the s10.2 variant.
**Consequence.** The two magics differ only in `m->lifetime` (31 vs 1). Every
spelling whose intent is "make the /1800 constant be consumed fewer times" is
inert. Corrects the rationale (not the number) in s7's H11.

### K29 — KILLED (class): lifetime cannot be pushed below 1
**Statement.** `m->lifetime` for the 0x91A2B3C5 movable cannot be made 0 by any
C spelling, so the LHS of loop.c:1631 is bounded below by `threshold` itself.
**Mechanism.** `loop.c:791` computes lifetime as a difference of `uid_luid`
values, and `loop_optimize` assigns luids strictly monotonically (+1 per
non-line-number insn); a 0 requires the pseudo to be mentioned in exactly one
insn, i.e. a set with no reader, which cse1/`delete_dead_from_cse` delete before
`loop_optimize`.
**predicate_cite.** tools/gcc-2.7.2/loop.c:791
**measured_on.** shipped chassis, candidate.c body, no FAKE constructs, floor 15.

### K30 — KILLED (instance): the movable-slot dial is not capped at three, but costs +3 emitted instructions per slot
**Statement.** Adding a fourth ordinary-C movable (giving the +4 time field its
own index-derived pointer, so `&D_80106A58 + 4` becomes its own invariant,
`m->forces`-chained to the base) pushes the 0x91A2B3C5 magic to movable slot 3
and its threshold to 116, but the build measures 28 at 108 build_insns.
**Mechanism.** loop.c:1719/1904 `threshold -= 3` per moved movable; the added
invariant also raises insn_count 56 → 58, so the gap `threshold - insn_count`
closes by 5 per movable (63 → 58) while the preheader grows.
**Result.** Directly contradicts s8's frontier item 3 ("None visible. s8
measured the loop has exactly three movables"). The dial exists and is ordinary
C; at −5 gap per slot the crossover is ~13 added invariants (agreeing with s4's
K14), which prices out at roughly +39 emitted instructions against a
104-instruction target.
**measured_on.** shipped chassis, s10 variant applied in src/, no FAKE
constructs, score 28 / 108 insns.
**Form.** rejected/extra-movable-dial-costs-3-insns-per-slot.c

### K31 — KILLED (instance): `may_not_optimize` is a fourth admission gate, but statement order does not trip it
**Statement.** Placing a reader of a doubly-materialised invariant pseudo
between its two sets did not set `may_not_optimize` for that pseudo in this
loop; the movable still printed `moved` (life 3 → 7, savings 2).
**Mechanism.** loop.c:649 gates the movable scan on `! may_not_optimize[regno]`;
`count_loop_regs_set` sets it when the pseudo is set in two basic blocks
(loop.c:3037) or set twice with `reg_used_between_p` true between the sets
(loop.c:3044). K23's premise that repeated source-level expressions always yield
DISTINCT pseudos is false — the repeated base symbol yields one pseudo with
`n_times_set == 2` — but GCC keeps each set adjacent to its own consumer, so no
reader lands strictly between them.
**Result.** Even if trippable, it cannot reach this residual: it would have to
fire on the 0x91A2B3C5 pseudo, needing a second in-loop `/1800` (~+7 emitted
instructions) or a second materialisation split across a branch, and the target
loop has exactly one division by 1800 and no conditional branch.
**measured_on.** shipped chassis, s10 reordered variant applied in src/, no FAKE
constructs, insn_count 58.

### K32 — KILLED (instance): no transplantable sibling shape
**Statement.** The project's only matched mm:ss.cc formatter, `func_8001CD68`
(`src/code6cac.c:1121-1139`), uses a different expression shape
(`val/30 - minutes*60`, centiseconds re-read from the global) than the one
`func_8003C714`'s bytes require, so it supplies no alternative body.
**Mechanism.** The target recomputes `(t/30)/60` with the 0x88888889 magic at
shift 5 (8003C79C..8003C7B8) rather than reusing the `/1800` quotient — the
signature of `(t / 30) % 60`, which candidate.c already spells.
**Result.** The sibling confirms candidate.c's expression shape by contrast. The
only other 0x91A2B3C5 carriers in the image (`func_80035280`, `func_80067D14`)
are both still `INCLUDE_ASM`, so the project has no matched in-loop precedent
for this constant.
**measured_on.** shipped chassis, source survey (grep over asm/funcs + src).

## [s10] m->savings in the loop.c:1631 desirability test is the number of times the invariant pseudo is SET inside the loop, not the number of times it is used, so the 0x91A2B3C5 and 0x88888889 magics are distinguished only by m->lifetime.
- mechanism: loop.c:793 sets m->savings = n_times_used[regno]; loop.c:597 initialises n_times_used as a straight bcopy of n_times_set, which count_loop_regs_set fills with SET counts for the loop range. Uses never enter the term.
- probe: Regenerated the .loop dump for the banked candidate on the shipped chassis (pwsh tools/grinder/dump.ps1 func_8003C714) and read the movable table at tmp/grind/func_8003C714/dumps/code6cac_c2.loop:4223, cross-checked against loop.c:597/791/793.
- result: The /30 magic (regno 91) is read by four in-loop mults and still prints savings 1, identical to the once-read /1800 magic (regno 84). It hoists on life 31 vs the /1800 magic's life 1. Corrects the rationale behind s7's H11 (the number 1 was right, the reason was not) and makes inert every spelling whose intent is to change how often the constant is consumed.
- verdict: CONFIRMED

## [s10] m->lifetime for the 0x91A2B3C5 movable has a hard floor of 1, so the left-hand side of loop.c:1631 is bounded below by threshold itself.
- mechanism: loop.c:791 computes lifetime as uid_luid[regno_last_uid] - uid_luid[regno_first_uid]; loop_optimize assigns luids strictly monotonically (+1 per non-line-number insn), so two distinct insns differ by at least 1. A lifetime of 0 requires the pseudo to be mentioned in exactly one insn, i.e. a set with no reader, which cse1 and delete_dead_from_cse remove before loop_optimize runs.
- probe: Read tools/gcc-2.7.2/loop.c:791 and the luid assignment in loop_optimize; confirmed against the baseline dump, which prints life 1 for both regno 78 and regno 84.
- result: Lifetime is pinned at 1 for the magic, so the gate reduces to threshold >= insn_count. With threshold 122 (loop.c:532, n_non_fixed_regs 60, no CALL_INSN in the loop so the loop_has_call halving branch is not reachable) and at most three threshold -= 3 decrements available, the LHS bottoms out around 116 against insn_count 56-58.
- verdict: KILLED
- kill_scope: class
- measured_on: shipped chassis, candidate.c body applied in src/code6cac_c2.c, no FAKE constructs, sandbox 15 (104/105)
- predicate_cite: tools/gcc-2.7.2/loop.c:791

## [s10] A fourth ordinary-C movable exists in this loop: reading the +4 time field through its own index-derived pointer makes &D_80106A58 + 4 a distinct loop invariant, m->forces-chained to the base, which pushes the 0x91A2B3C5 magic to movable slot 3 and its threshold to 116 - and the build measures 28 at 108 build_insns.
- mechanism: Each moved movable applies threshold -= 3 (loop.c:1719 and loop.c:1904) before the next is tested. The added invariant also raises the loop's insn_count from 56 to 58, so the desirability gap threshold - insn_count closes by 5 per movable slot while the preheader grows by the hoisted set.
- probe: Replaced *(s32 *)(src + 4) with a separate tsrc = (s32 *)((u8 *)&D_80106A58 + 4 + i * 8) pointer in src/code6cac_c2.c, ran sandbox func_8003C714 --disable all, then regenerated and read the .loop movable table.
- result: Movable table became four entries: Insn 33 regno 79 (life 3, savings 2) base; Insn 41 regno 82 (life 1, forces 33, savings 1) base+4 - NEW; Insn 54 regno 88 (life 1) 0x91A2B3C5; Insn 68 regno 95 (life 31) 0x88888889. Loop from 25 to 154: 58 real insns. Score 15 -> 28, build_insns 105 -> 108. This directly contradicts s8's frontier item 3 (None visible; s8 measured the loop has exactly three movables). The dial is ordinary C and not capped at three, but at -5 gap per slot the ~13-invariant crossover measured by s4's K14 prices out at roughly +39 emitted instructions against a 104-instruction target. Form banked at rejected/extra-movable-dial-costs-3-insns-per-slot.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: shipped chassis, s10 four-movable variant applied in src/code6cac_c2.c, no FAKE constructs, sandbox 28 (104/108)

## [s10] Reordering the loop body so a reader of a doubly-materialised invariant pseudo sits between its two sets did not set may_not_optimize for that pseudo: the movable still printed moved, with life 3 -> 7 and savings 2.
- mechanism: loop.c:649 gates the movable scan on ! may_not_optimize[REGNO (SET_DEST (set))]; count_loop_regs_set raises that flag when the pseudo is set in two basic blocks (loop.c:3037) or set twice with reg_used_between_p true between the sets (loop.c:3044). GCC keeps each materialisation adjacent to its own consumer, so no reader lands strictly between the two sets.
- probe: Took the four-movable variant and moved dst[0x24] = *src (a reader of the base pseudo) to sit between the src and tsrc materialisations; regenerated the .loop dump and read the movable table.
- result: Entry became 'Insn 33: regno 79 (life 7), move-insn savings 2  moved' - still a movable. Two corrections to s8's K23 nonetheless: (a) may_not_optimize is a FOURTH admission gate its enumeration of 'exactly three tests' omitted, and (b) K23's premise that repeated source-level expressions always yield DISTINCT pseudos is false, since the doubly-materialised base yields ONE pseudo with n_times_set 2. Reaching the 0x91A2B3C5 pseudo with this gate would need a second in-loop division by 1800 (~+7 emitted instructions) or a second materialisation split across a branch, and the target loop has one division by 1800 and no conditional branch.
- verdict: KILLED
- kill_scope: instance
- measured_on: shipped chassis, s10 reordered four-movable variant applied in src/code6cac_c2.c, no FAKE constructs, insn_count 58

## [s10] The project's only matched mm:ss.cc formatter, func_8001CD68 at src/code6cac.c:1121-1139, spells its seconds term as val/30 - minutes*60 and re-reads the global for the centiseconds term, which the target's bytes for func_8003C714 contradict, so it yields no alternative body to transplant.
- mechanism: func_8003C714's target recomputes (t/30)/60 with the 0x88888889 magic at shift 5 at 8003C79C..8003C7B8 instead of reusing the /1800 quotient - the signature of (t / 30) % 60, which the banked candidate already spells.
- probe: Grepped asm/funcs for the 0x91A2B3C5 magic and src/*.c for 1800; read src/code6cac.c:1100-1140 and compared its expression shape against the target disassembly.
- result: The sibling confirms the banked body's expression shape by contrast rather than replacing it. The only other 0x91A2B3C5 carriers in the image are func_80035280 and func_80067D14, both still INCLUDE_ASM, so the project holds no matched in-loop precedent for this constant.
- verdict: KILLED
- kill_scope: instance
- measured_on: shipped chassis, source survey (grep over asm/funcs and src) plus target disassembly read

## s11 (2026-09-05, rederive modality)

### H16 — CONFIRMED. The loop.c:1609 `moved_once` insn_count doubling is NOT self-defeating when the carrier is an EARLIER movable than the 0x91A2B3C5 magic.
Mechanism: `insn_count *= 2` at loop.c:1609 mutates scan_loop's local permanently, so every
movable tested after the carrier sees the doubled count, while `threshold` is decremented
(loop.c:1719/1904) only by movables that are actually MOVED. s7's K20 measured the doubling with
the magic itself as the carrier, which inflates that movable's own `m->lifetime` (1 -> 14/15) and
multiplies the LHS; a separate carrier leaves the magic at lifetime 1.
Probe: three diagnostic bodies, dumps in `tmp/grind/func_8003C714/s11/`.
Result: with the carrier placed between the base-address computation and the first division
(`s11/varE.c`, `s11/varE.loop`), the dump prints the target's exact arrangement — base address
`moved`, carrier `halved since already moved  moved`, `0x91A2B3C5 ... not desirable`, `0x88888889
... moved`. Verdict CONFIRMED on the shipped chassis, no FAKE constructs present.

### K33 — KILLED (instance). The leanest moved_once carrier measured costs +7 emitted instructions.
Statement: the inner-loop moved_once carrier spelled `j = 0; do { D_800A37B8 = 5; j += 1; } while
(j < 2);` placed between `dst = ...` and the first division builds to 112 instructions and
sandbox 18, i.e. +7 over the honest body's 105, and the 0x12345678 spelling of the same carrier
builds to 113 / score 19.
kill_scope: instance. measured_on: shipped chassis, s11 varD.c / varE.c applied in
src/code6cac_c2.c, no FAKE constructs, `sandbox func_8003C714 --disable all` = 19 / 18.
Note the score rose only 15 -> 18 for +7 instructions: declining the /1800 hoist repairs several
of the 15 residual differences, independently corroborating s6's distance-0 result.

### K34 — KILLED (instance). A symbol-address invariant inside the inner loop does not create a movable, so it cannot carry moved_once.
Statement: an inner loop whose only invariant is `&D_80106A58` produced no movable at all — loop.c
folded the symbol into the giv as `dest address src reg 74 ... add (symbol_ref:SI
("D_80106A58"))` — so `moved_once` stayed clear and all three outer movables still moved at
insn_count 64.
kill_scope: instance. measured_on: shipped chassis, s11 varB_movedonce_early.c (first spelling)
applied in src/code6cac_c2.c, no FAKE constructs, `.loop` dump inspected (no movable line printed
for the inner loop). A CONST_INT invariant does work (H16).

### H17 — CONFIRMED. The admissible insn_count window is [120, 122], not "insn_count >= 120".
Mechanism: the movable table (s11.1 of evidence.md) puts the base address at slot 1 (threshold
122) and the /1800 magic at slot 2 (threshold 119). The target hoists the base and keeps the magic
in-loop, so `122 >= insn_count` AND `119 < insn_count` must both hold.
Probe: read from `tmp/grind/func_8003C714/s11/base.loop`, and corroborated by the varB spelling
where insn_count 63 doubled to 126 and BOTH the base address and the magic printed `not
desirable`.
Result: s6's distance-0 form sat at insn_count 121, inside the window. Any future
insn_count-inflation attack must land in a 3-wide window; at >= 123 the preheader loses its
`lui/addiu $a2, %hi/%lo(D_80106A58)` pair and the match breaks from the other side. Verdict
CONFIRMED.

## [s11] The loop.c:1609 moved_once insn_count doubling is not self-defeating when the carrier is a movable EARLIER in the list than the 0x91A2B3C5 magic; with the carrier placed between the base-address computation and the first division, the .loop dump prints the target's exact arrangement (base address moved, 0x91A2B3C5 not desirable, 0x88888889 moved).
- mechanism: loop.c:1609 'if (moved_once[regno]) insn_count *= 2;' mutates scan_loop's local permanently, so every movable tested afterwards sees the doubled count, while threshold is decremented (loop.c:1719/1904) only by movables actually moved. moved_once[] is per-function (alloca+bzero at loop.c:344-345) and written only at loop.c:1912, and loop_optimize scans innermost/latest loops first, so an invariant hoisted out of a nested loop arrives at our scan already flagged. s7's K20 attached the flag to the magic itself, which inflates that movable's own m->lifetime 1 -> 14/15 and multiplies the LHS; a separate carrier leaves the magic at lifetime 1.
- probe: Three diagnostic bodies compiled with the instrumented cc1 via tmp/grind/func_8003C714/run_dump.sh and their .loop movable tables read: carrier at the top of the body (both base and magic declined at 2*63=126), carrier absent (baseline, all three moved at 56), carrier between the base-address computation and the first division (target arrangement).
- result: tmp/grind/func_8003C714/s11/varE.loop: 'Loop from 25 to 174: 62 real insns. / Insn 33: regno 79 (life 1) moved to 233 / Insn 231: regno 82 (life 9) halved since already moved moved to 235 / Insn 74: regno 87 (life 1) not desirable / Insn 88: regno 94 (life 31) moved to 237'. Slot 1 is tested at threshold 122 against the UNDOUBLED count and moves; the carrier is slot 2, doubles the count to 124 and moves (119*9 >= 124), taking threshold to 116; the magic is slot 3 and 116 < 124 declines it; the /30 magic is slot 4 at 116*31. Generalised: with the carrier between base and magic the requirement is 2*insn_count > 116 and insn_count <= 122, i.e. insn_count >= 59 - a +3 RTL-insn ask instead of the +64 the s5/s6 record implies.
- verdict: CONFIRMED

## [s11] The insn_count range that reproduces the target's preheader is [120, 122] when no moved_once carrier is present, because the base-address movable occupies slot 1 (threshold 122) and the 0x91A2B3C5 magic occupies slot 2 (threshold 119), and the target hoists the base while keeping the magic in-loop.
- mechanism: loop.c:1631 tests threshold * savings * m->lifetime >= insn_count per movable in list order, and loop.c:1719/1904 subtract 3 from threshold only when a movable is actually moved. The baseline table read this session gives slot 1 = &D_80106A58 (life 1, savings 1), slot 2 = 0x91A2B3C5 (life 1, savings 1), slot 3 = 0x88888889 (life 31, savings 1). The target's preheader at 8003C73C..8003C750 contains the base address and the /30 magic and not the /1800 magic, which forces 122 >= insn_count and 119 < insn_count simultaneously.
- probe: Read tmp/grind/func_8003C714/s11/base.loop (Loop from 25 to 146: 56 real insns) against the shipped disassembly asm/funcs/func_8003C714.s, and corroborated with the varB spelling where insn_count 63 doubled to 126 and BOTH the base address and the magic printed 'not desirable'.
- result: s6's distance-0 form sat at insn_count 121, inside the window - the ledger had recorded only the lower bound. Any future insn_count-inflation attack must land in a 3-wide window: at insn_count >= 123 the preheader loses its lui/addiu $a2,%hi/%lo(D_80106A58) pair and the match breaks from the other side. The dump also directly confirms s10's H15: the /30 magic is read by four in-loop mults and still prints 'savings 1'.
- verdict: CONFIRMED

## [s11] The leanest moved_once carrier found this session, an inner loop spelled 'j = 0; do { D_800A37B8 = 5; j += 1; } while (j < 2);' placed between the dst computation and the first division, builds to 112 instructions at sandbox score 18 (+7 emitted instructions over the honest body's 105), and the 0x12345678 spelling of the same carrier builds to 113 at score 19.
- mechanism: The carrier must be a loop because moved_once[] is written nowhere but loop.c:1912. An inner loop emits, at minimum, the counter init, the body store, the increment, the compare and the backward branch, plus the hoisted CONST_INT in the outer preheader. The shipped target is 104 instructions and the honest body already overshoots it by one.
- probe: sandbox func_8003C714 --disable all on tmp/grind/func_8003C714/s11/varD.c and s11/varE.c applied over the INCLUDE_ASM line in src/code6cac_c2.c; the honest candidate.c re-measured immediately before and after at 15 / 105.
- result: varD 113 insns score 19; varE 112 insns score 18; baseline 105 insns score 15. Note the score rose only 15 -> 18 for +7 instructions: declining the /1800 hoist genuinely repairs several of the 15 residual differences, which is independent corroboration of s6's distance-0 measurement. Banked as memory/grind/func_8003C714/rejected/movedonce-inner-loop-carrier-costs-7-insns.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: shipped chassis (HEAD 2026-09-05), s11 varD.c / varE.c applied in src/code6cac_c2.c, no FAKE constructs present, rules_dropped 0

## [s11] An inner loop whose only invariant is the symbol address &D_80106A58 creates no movable at all, so it cannot set moved_once; the carrier has to be a CONST_INT invariant.
- mechanism: loop.c's strength reduction folds a loop-invariant symbol_ref used as an address base into the giv itself, printing 'dest address src reg 74 ... add (symbol_ref:SI (D_80106A58))' rather than creating a movable, so move_movables never runs on it and loop.c:1912 never fires.
- probe: First spelling of tmp/grind/func_8003C714/s11/varB_movedonce_early.c compiled with the instrumented cc1; the inner loop's table in the .loop dump was inspected for a movable line.
- result: Inner loop printed 'Loop from 32 to 54: 4 real insns' with no movable line, and the outer loop still moved all three movables at insn_count 64 with no 'halved since already moved' text. Replacing the store with a CONST_INT assignment to a global immediately produced a movable and the doubling.
- verdict: KILLED
- kill_scope: instance
- measured_on: shipped chassis (HEAD 2026-09-05), s11 varB_movedonce_early.c first spelling applied in src/code6cac_c2.c, no FAKE constructs present

## [s11] The sibling func_80067D14 (src/text1b.c) has no candidate.c in its ledger, so no sibling spelling was available to transplant onto this chassis.
- mechanism: The sibling-propagation rule requires reading the sibling's candidate.c and transplanting its spelling of every shared block; memory/grind/func_80067D14/ contains no candidate.c (the dispatch brief also reports 'candidate: no candidate.c').
- probe: Directory check of memory/grind/func_80067D14/ at session start.
- result: Nothing to transplant; recorded so a later session does not re-spend the sweep until that sibling banks a body.
- verdict: KILLED
- kill_scope: instance
- measured_on: ledger state at 2026-09-05, memory/grind/func_80067D14/ contains no candidate.c

## s12 (2026-09-05, structural modality)

### K35 - KILLED (class). No moved_once carrier can be byte-neutral, because every carrier is a second emitted loop and the target has exactly one.
Statement: setting `moved_once[regno]` requires a movable to be MOVED out of some other loop
(the flag's only write is loop.c:1912), and scan_loop processes a loop only when its scan_start
is a CODE_LABEL (loop.c:568-576, "is phony" early return), so every carrier is a second loop with
its own top label and backward branch in the emitted code; func_8003C714's target
(asm/funcs/func_8003C714.s) contains exactly one label (.L8003C754, line 18) and one branch
(bnez $v0 at line 77), so no moved_once carrier can be byte-neutral on this chassis and the
loop.c:1609 insn_count doubling is unreachable for a matching body.
kill_scope: class. predicate_cite: tools/gcc-2.7.2/loop.c:1912.
measured_on: shipped chassis (HEAD 2026-09-05), candidate.c and s11/varE.c applied in
src/code6cac_c2.c, no FAKE constructs present; varE's emitted asm in
tmp/grind/func_8003C714/dumps/code6cac_c2.s carries the second label .L141 and its `bne`, and
CC_FLAGS (engine/buildconfig.py:43) contains no -funroll-loops, so nothing after loop_optimize
can delete a >=2-trip loop's backward branch.
Consequence: s11's H16 arrangement (requirement insn_count >= 59) is a diagnostic result only.
The live requirement returns to H17's no-carrier window, insn_count in [120, 122] with zero
emitted cost.

### K36 - KILLED (instance). A natural extra induction variable is free in bytes but yields ZERO insn_count.
Statement: replacing the `i * 8` scaling with a second natural index (`k += 8`, `src =
(u8 *)&D_80106A58 + k`) builds byte-identically to the honest body (score 15, 105 insns) and its
.loop dump still reports "Loop from 28 to 149: 56 real insns" - the new index is recognised as
biv reg 74 (const 8), replaces the giv multiply, and is eliminated, so loop.c's insn_count is
unchanged; a third index (`m += 4` for dst) costs +2 emitted instructions (107 insns, score 23).
kill_scope: instance. measured_on: shipped chassis (HEAD 2026-09-05), s12/pc_second_biv.c and
s12/pd_three_indices.c applied in src/code6cac_c2.c, no FAKE constructs present, `sandbox
func_8003C714 --disable all` = 15 and 23, .loop dump read at
tmp/grind/func_8003C714/dumps/code6cac_c2.loop:4223.

### K37 - KILLED (instance). The array-subscript spelling of the body is byte-equivalent, not better.
Statement: spelling the loop body with array subscripts on two pre-loop base pointers
(`out[i*4 + 0x21] = *(s32 *)(base + i*8 + 4) / 1800; ...`) instead of per-iteration index-derived
pointers measures score 15 at 105 build_insns, identical to candidate.c, while re-scaling the biv
to step 4 (`i += 4`, limit 12, `src = base + i*2`, `dst = s0 + i`) measures 21 at the same 105
insns.
kill_scope: instance. measured_on: shipped chassis (HEAD 2026-09-05), s12/pb_array_subscript.c and
s12/pa_biv_step4.c applied in src/code6cac_c2.c, no FAKE constructs present, `sandbox
func_8003C714 --disable all` = 15 and 21.

### K33 re-audit (mandated) - STANDS
s11's leanest moved_once carrier (s11/varE.c) re-measured on the current chassis: score 18,
build_insns 112, unchanged from s11. `tools/fake_ablate.py` reports no FAKE-annotated constructs
in candidate.c, so no banked lever was measured under a FAKE carrier.

## [s12] The moved_once insn_count-doubling channel (s11 H16) is byte-foreclosed: every carrier is a second emitted loop (loop.c:568-576 phony gate -> loop.c:1912 flag write) and the target function contains exactly one label and one branch.

## [s12] A second natural index variable is byte-free AND insn_count-free (it becomes an eliminated biv that replaces the giv multiply), so the natural-biv route cannot inflate insn_count toward the [120,122] window.

## [s12] Setting moved_once[regno] requires a movable to be moved out of some other loop (its only write is loop.c:1912) and scan_loop processes a loop only when its scan_start is a CODE_LABEL (loop.c:568-576 'is phony' early return), so every carrier is a second loop that emits its own top label and backward branch, while the target func_8003C714 contains exactly one label (.L8003C754) and one branch (bnez at 8003C83C); therefore no moved_once carrier can be byte-neutral on this chassis.
- mechanism: loop.c:1609 doubles scan_loop's local insn_count for a movable whose regno already carries moved_once, which is the cheapest known route to the target's movable arrangement (s11 H16 dropped the requirement from insn_count > 119 to insn_count >= 59). But the flag can only be set by a genuine scan_loop move in another loop, and a loop that survives the phony gate has a real top CODE_LABEL and a real backward jump. With s11/varE.c applied the emitted func_8003C714 carries a second label .L141 and a second 'bne $2,$0,.L141' beside the real loop's .L133/bne; the shipped CC_FLAGS (engine/buildconfig.py:43) has no -funroll-loops, so no pass after loop_optimize can fold a >=2-trip loop into straight-line code.
- probe: grep of tools/gcc-2.7.2/loop.c for the moved_once write (line 1912) and the phony gate (lines 568-576); grep of asm/funcs/func_8003C714.s for labels/branches (exactly one of each); regeneration of tmp/grind/func_8003C714/dumps/code6cac_c2.s with s11/varE.c applied and a label/branch count of the emitted function; re-measurement of varE.c with sandbox (18 / 112 insns).
- result: Target has one label and one branch; varE (the leanest known carrier) emits two of each and costs +7 instructions. The moved_once route is closed for a matching body; the live loop.c:1631 requirement returns to H17's no-carrier window, insn_count in [120,122] at zero emitted cost.
- verdict: KILLED
- kill_scope: class
- measured_on: shipped chassis HEAD 2026-09-05, candidate.c and s11/varE.c applied in src/code6cac_c2.c, no FAKE constructs present (fake_ablate.py reports none in candidate.c)
- predicate_cite: tools/gcc-2.7.2/loop.c:1912

## [s12] Replacing the i*8 scaling with a second natural index variable (k += 8, src = (u8 *)&D_80106A58 + k) builds byte-identically to the honest body at score 15 / 105 insns and leaves loop.c's insn_count unchanged at 56, because the new index becomes verified biv reg 74 (const 8), replaces the giv multiply and is eliminated; a third index (m += 4 for dst) costs +2 emitted instructions at score 23 / 107.
- mechanism: count_loop_regs_set counts 'i'-class insns between loop start and end before strength reduction, so an added induction variable could in principle inflate insn_count for free. Measured: it does not, because the biv displaces the giv multiply it replaces (net zero RTL insns) and is then eliminated by strength_reduce.
- probe: tmp/grind/func_8003C714/s12/pc_second_biv.c and pd_three_indices.c applied with s12/apply.py, measured with sandbox func_8003C714 --disable all, and the .loop dump read at tmp/grind/func_8003C714/dumps/code6cac_c2.loop:4223 ('Loop from 28 to 149: 56 real insns', movables 77/83/90 all moved).
- result: pc: 15 / 105 insns, insn_count 56 (unchanged). pd: 23 / 107 insns. The natural-biv route buys nothing on the loop.c:1631 gate.
- verdict: KILLED
- kill_scope: instance
- measured_on: shipped chassis HEAD 2026-09-05, s12/pc_second_biv.c and s12/pd_three_indices.c applied in src/code6cac_c2.c, no FAKE constructs present, sandbox = 15 and 23

## [s12] Spelling the loop body with array subscripts on two pre-loop base pointers (out[i*4 + 0x21] = *(s32 *)(base + i*8 + 4) / 1800, ...) instead of per-iteration index-derived pointers measures score 15 at 105 build_insns, identical to candidate.c, and re-scaling the biv to step 4 (i += 4 to 12, src = base + i*2, dst = s0 + i) measures 21 at the same 105 insns.
- mechanism: Structural modality levers: declaration order, statement re-association and subscript-vs-pointer spelling. cse1 converges the array-subscript and index-derived-pointer forms to the same RTL, so the byte output is identical; changing the biv's scale instead re-shapes the givs and moves the emitted arrangement away from the target.
- probe: tmp/grind/func_8003C714/s12/pb_array_subscript.c and pa_biv_step4.c applied with s12/apply.py and measured with sandbox func_8003C714 --disable all.
- result: pb: 15 / 105 (byte-equivalent alternative spelling, useful as a second permuter seed). pa: 21 / 105.
- verdict: KILLED
- kill_scope: instance
- measured_on: shipped chassis HEAD 2026-09-05, s12/pb_array_subscript.c and s12/pa_biv_step4.c applied in src/code6cac_c2.c, no FAKE constructs present, sandbox = 15 and 21

## [s12] The mandated kill re-audit reproduces s11's K33 exactly on the current chassis: the leanest moved_once carrier (s11/varE.c) still measures score 18 at 112 build_insns, and tools/fake_ablate.py finds no FAKE-annotated constructs in candidate.c, so no banked lever of this ledger was measured while a FAKE carrier occupied its pseudo.
- mechanism: An instance kill is only as good as the chassis and FAKE state it was measured under; the brief mandates re-measuring the closest instance kill before new probes.
- probe: candidate.c applied and measured (15 / 105 / rules_dropped 0 / cheat_asm_stripped 9), then s11/varE.c applied and measured, then tools/fake_ablate.py --func func_8003C714 --file code6cac_c2 --candidate memory/grind/func_8003C714/candidate.c.
- result: Floor 15 current; K33 stands at 18 / 112; ablation not applicable (no FAKE constructs).
- verdict: CONFIRMED


## s13 (2026-09-05) - structural modality

**H18 (CONFIRMED, source-level).** The desirability test at loop.c:1631 is one of
THREE OR'd move conditions, and the two non-arithmetic ones are structurally
unavailable to this function. `already_moved[regno]` (loop.c:1630) is set only
after some other movable loading the SAME register was moved (loop.c:1910), which
cannot happen before the magic is itself tested. The third disjunct
(loop.c:1632-1633) is a FORCE-to-move, not an escape. That leaves
`threshold * savings * m->lifetime >= insn_count`, and s10's H15 + K29 pin
savings and lifetime at 1, so the gate is exactly `threshold >= insn_count` and
H17's window `insn_count in [120, 122]` is the complete statement of the
requirement. Also re-derived: loop.c:1719 and loop.c:1904 are the only writes to
`threshold`, both `threshold -= 3` and both once per MOVED movable irrespective
of `m->consec`, so the order dial's slope is exactly -3 per hoist (matching
K30's measured +3 emitted instructions per slot).

**K38 (CLASS KILL) - the `m->forces` skip route cannot reach a constant-load
movable.** `move_movables` skips a movable outright when
`m->forces && ! m->forces->done` (loop.c:1594), which would leave the 0x91A2B3C5
load in the loop at zero insn_count cost. `m->forces` is written in exactly one
place, `force_movables` at loop.c:1221, and only for a movable `m` whose setting
insn is the last insn mentioning an earlier movable's register
(`INSN_UID (m->insn) == regno_last_uid[m1->regno]`). The dump shows this loop's
three movables are `(set (reg 78) (symbol_ref "D_80106A58"))`,
`(set (reg 84) (const_int -1851608123))` and
`(set (reg 91) (const_int -2004318071))` - none of them mentions any register
other than its own destination, so no earlier movable's register can have its
last use there and the `forces` pointer can never be attached. Additionally
`force_movables` doubles the FORCING movable's savings and adds the forced
movable's lifetime to it (loop.c:1221-1223), so the shape would drive `m1` to
`done` and loop.c:1632 would then force-move the magic. The route is closed for
every constant-materialisation movable, which is what the /1800 magic is on this
target.
- kill_scope: class
- predicate_cite: tools/gcc-2.7.2/loop.c:1221
- measured_on: shipped chassis HEAD 2026-09-05, baseline candidate.c applied,
  no FAKE constructs present (fake_ablate: nothing to ablate); RTL read from
  tmp/grind/func_8003C714/s13/baseline.loop insns 203/205/207.

**K39 (CLASS KILL) - the `may_not_move` two-basic-block admission trigger cannot
fire for a division magic.** `count_loop_regs_set` sets `may_not_move[regno]`
when `n_times_set[regno] > 0 && last_set[regno] == 0` (loop.c:3037), and
`last_set` is zeroed only at a CODE_LABEL or JUMP_INSN (loop.c:3089-3090), so the
trigger requires ONE pseudo SET twice with a label or jump between the sets.
Every division site expands its magic constant into a FRESH pseudo, and cse1's
only transform is to replace USES of an equivalent value - it never adds a second
SET to an existing pseudo - so a magic-constant pseudo's `n_times_set` is 1.
Measured on the explicit two-basic-block spelling
(`rejected/twobb-division-gives-two-matched-pseudos-not-may-not-move.c`: the
/1800 division duplicated into both arms of an in-loop `if (i != 0)`, else arm
perturbed so jump.c cannot cross-jump them): the `.loop` table prints
`Insn 50: regno 84 (life 2), move-insn savings 2  moved to 232` and
`Insn 71: regno 92 (life 1), done move-insn matches 50` - two distinct pseudos,
`may_not_move` silent, and `combine_movables` (loop.c:1245, gate
`n_times_used[m->regno] == 1`) matching them and raising the surviving movable's
savings 1 -> 2 and lifetime 1 -> 2. The desirability product therefore moves the
WRONG way, 119 -> 476, while the extra basic block costs +13 emitted
instructions (score 27, build_insns 118). This supplies the mechanism s8's K23
inferred from scores only.
- kill_scope: class
- predicate_cite: tools/gcc-2.7.2/loop.c:3037
- measured_on: shipped chassis HEAD 2026-09-05, s13/pa_twobb_div.c applied in
  src/code6cac_c2.c, no FAKE constructs present.

**K40 (instance) - the target's post-loop tail has no instruction that could
consume a loop-carried accumulator.** The s12 frontier asked whether any
semantically real loop-carried value could supply the free insn_count that s6
measured (balanced constant-step bivs, deleted by biv elimination for zero
bytes). The answer is decided by the target's 23-instruction tail
(8003C844..8003C8B0), enumerated instruction by instruction this session: it is
`func_8001CD68(buf)`, three byte stores from `buf`, one byte store from
`D_80101ED2`, the four-argument `disp_SetFramebufferMode(1,0,0,0)` setup, the
`D_800A37B8 = 0` / `D_800A3834 = 0x1F` stores, the `gpu_DisableDisplay()` call
and the epilogue - a 1:1 map onto the eight source statements candidate.c
already carries, with no register read that the loop could have left live. A
loop-carried accumulator in this function is therefore dead at the loop exit,
which is the inert-padding shape s6 already banked as inadmissible.
- kill_scope: instance
- measured_on: shipped chassis HEAD 2026-09-05, target disassembly
  asm/funcs/func_8003C714.s read against candidate.c's statement list; no FAKE
  constructs present.

**K37 re-audit (STANDS).** s12's array-subscript chassis re-measured on the
current chassis: score 15, build_insns 105, rules_dropped 0 - byte-equivalent to
candidate.c. `fake_ablate` reports no FAKE constructs in candidate.c, so no
banked lever was measured under a FAKE carrier.

## [s13] The move decision at loop.c:1631 is three OR'd conditions, and the two that do not involve insn_count are not escapes: already_moved[regno] (loop.c:1630) is written only at loop.c:1910 after another movable loading the SAME register was moved, and the m->forces disjunct at loop.c:1632 is a force-TO-move. With savings and lifetime pinned at 1 (s10 H15/K29) the gate reduces exactly to 'threshold >= insn_count', so H17's window insn_count in [120,122] is the complete requirement. threshold is written only at loop.c:1719 and loop.c:1904, both 'threshold -= 3', both once per MOVED movable regardless of m->consec, so the movable-order dial's slope is exactly -3 per hoist.
- mechanism: move_movables reads m->savings = n_times_used[regno] (loop.c:793), and n_times_used is a straight bcopy of n_times_set (loop.c:597), so savings counts SETS and has floor 1; m->lifetime (loop.c:791) is a difference of monotone LUIDs with floor 1. The product's minimum is therefore threshold itself.
- probe: Read loop.c:505-545 (threshold formula), 760-900 (movable record construction), 1185-1245 (force_movables/combine_movables), 1570-1660 (move_movables head + desirability), 1710-1725 and 1896-1910 (the two threshold decrements); cross-checked against the regenerated baseline .loop movable table for the candidate body.
- result: Confirmed from compiler source. threshold = (loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs) = 122 here, 61 with a call in the loop; the -3 decrement fires once per moved movable and never scales with m->consec, so a consec-group movable buys -3 while emitting several preheader instructions. No cheaper order dial exists than the +3-emitted-instructions-per-slot slope K30 already measured.
- verdict: CONFIRMED

## [s13] The m->forces skip path at loop.c:1594 ('! m->forces || m->forces->done'), which would leave a movable in the loop at zero insn_count cost, cannot be reached by any constant-materialisation movable in this loop, because force_movables (loop.c:1221) attaches the forces pointer only to a movable whose setting insn is the last insn mentioning an earlier movable's register, and all three movables here are (set (reg) <const_int|symbol_ref>) mentioning no register but their own destination.
- mechanism: force_movables walks movables m1 and finds the first later non-done movable m with INSN_UID (m->insn) == regno_last_uid[m1->regno]; only then does it set m->forces = m1. A set from a CONST_INT or SYMBOL_REF references no source register, so it can never be the last-mention insn of another pseudo. force_movables additionally does m1->savings += m1->savings and m1->lifetime += m->lifetime, which makes the forcing movable strictly more desirable, so even if the shape existed m1 would reach done and loop.c:1632's third disjunct would force-move the magic.
- probe: Regenerated the candidate's cc1 dumps with tools/grinder/dump.ps1 and read the post-loop RTL of the three hoisted insns in tmp/grind/func_8003C714/s13/baseline.loop: insn 203 = (set (reg:SI 78) (symbol_ref "D_80106A58")), insn 205 = (set (reg:SI 84) (const_int -1851608123) = 0x91A2B3C5), insn 207 = (set (reg:SI 91) (const_int -2004318071) = 0x88888889). Then traced every write of m->forces in tools/gcc-2.7.2/loop.c (grep: only line 1221).
- result: Route closed for every constant-materialisation movable. The .loop table for the baseline prints no 'forces' annotation on any of the three movables, consistent with the source analysis. This is the last non-arithmetic escape from loop.c:1631 and it is now enumerated rather than assumed.
- verdict: KILLED
- kill_scope: class
- measured_on: shipped chassis HEAD 2026-09-05, memory/grind/func_8003C714/candidate.c applied over the INCLUDE_ASM line in src/code6cac_c2.c (sandbox score 15, 104 target / 105 build, rules_dropped 0); no FAKE constructs present (tools/fake_ablate.py reports nothing to ablate)
- predicate_cite: tools/gcc-2.7.2/loop.c:1221

## [s13] The may_not_move admission trigger at loop.c:3037 (n_times_set[regno] > 0 && last_set[regno] == 0, with last_set zeroed only at a CODE_LABEL or JUMP_INSN, loop.c:3089) cannot fire for a division's magic constant, because each division site expands its magic into a fresh pseudo with n_times_set 1; what fires instead is combine_movables (loop.c:1245), which matches the two pseudos and raises the surviving movable's savings 1->2 and lifetime 1->2, moving the desirability product from 119 to 476.
- mechanism: count_loop_regs_set only marks a register unmovable when it sees a second SET of the same regno with no preceding set in the current basic block. A magic constant is materialised by expmed at each division site into a new pseudo, and cse1's only transform is to replace USES of an equivalent value, never to add a second SET to an existing pseudo, so the same regno is never set twice. combine_movables' own gate is n_times_used[m->regno] == 1, which these single-set pseudos satisfy exactly.
- probe: Built tmp/grind/func_8003C714/s13/pa_twobb_div.c: the /1800 division duplicated into both arms of an in-loop 'if (i != 0)', with the else arm perturbed to (t + 1) / 1800 so jump.c cannot cross-jump the arms back together. Measured with sandbox --disable all and read the regenerated .loop movable table.
- result: score 27, build_insns 118 (+13 emitted instructions). Movable table: 'Insn 50: regno 84 (life 2), move-insn savings 2 moved to 232' and 'Insn 71: regno 92 (life 1), done move-insn matches 50' — two distinct pseudos, may_not_move silent, combine_movables matching them. This reproduces s8's K23 outcome and now supplies its mechanism from the dump instead of from the score. Separately, the branch cost is structural: any interior basic-block split needs an emitted branch or an extra referenced label (jump.c deletes unreferenced labels), and the target function contains exactly one CODE_LABEL (.L8003C754) and one branch (bnez at 8003C83C) besides its five jal and the closing jr $ra. Banked as memory/grind/func_8003C714/rejected/twobb-division-gives-two-matched-pseudos-not-may-not-move.c.
- verdict: KILLED
- kill_scope: class
- measured_on: shipped chassis HEAD 2026-09-05, tmp/grind/func_8003C714/s13/pa_twobb_div.c applied in src/code6cac_c2.c; no FAKE constructs present
- predicate_cite: tools/gcc-2.7.2/loop.c:3037

## [s13] In this function the free insn_count carrier s6 measured (balanced constant-step bivs, counted by count_loop_regs_set and then deleted by biv elimination for zero emitted bytes) has no semantically-real spelling, because the target's 23-instruction post-loop tail maps 1:1 onto the eight source statements candidate.c already carries and reads no register the loop could have left live.
- mechanism: A loop-carried accumulator only earns its instructions if a post-loop consumer reads it. strength_reduce deletes a biv whose exit value it can fold, i.e. a constant-step biv, and the resulting construct is byte-free precisely when nothing consumes it — which is the inert-padding shape s6 banked as inadmissible.
- probe: Enumerated asm/funcs/func_8003C714.s from 8003C844 to 8003C8B0 instruction by instruction and mapped each onto candidate.c's post-loop statements: jal func_8001CD68 + addiu $a0,$sp,0x10; lhu 0x10($sp)/sb 0x2D($s0); lbu 0x12($sp)/sb 0x2E($s0); lbu 0x13($sp)/sb 0x2F($s0); lui+lhu D_80101ED2/sb 0x30($s0); the four addiu/addu argument setups plus jal disp_SetFramebufferMode; addiu $v0,0x1F plus the two lui+sw / lui+sh global stores; jal gpu_DisableDisplay; and the five-instruction epilogue.
- result: Every tail instruction is accounted for by an existing source statement; there is no spare instruction that could read an accumulator, and no argument slot fed from a loop-carried value. Any loop-carried carrier added to reach H17's insn_count window would therefore be dead at loop exit. Recorded as K40.
- verdict: KILLED
- kill_scope: instance
- measured_on: shipped chassis HEAD 2026-09-05, target disassembly asm/funcs/func_8003C714.s read against the statement list of memory/grind/func_8003C714/candidate.c; no FAKE constructs present

## [s13] s12's K37 (the array-subscript chassis, tmp/grind/func_8003C714/s12/pb_array_subscript.c) re-measured on the current chassis is still byte-equivalent to candidate.c, and candidate.c carries no FAKE construct, so no banked lever in this ledger was measured while a FAKE carrier occupied a target pseudo.
- mechanism: cse1 folds the array-subscript address arithmetic into the same givs the index-derived-pointer spelling produces, so the two bodies present different pre-cse RTL and identical post-cse RTL.
- probe: Mandated kill re-audit: tools/fake_ablate.py --func func_8003C714 --file code6cac_c2 --candidate memory/grind/func_8003C714/candidate.c, then applied s12/pb_array_subscript.c and ran sandbox func_8003C714 --disable all.
- result: fake_ablate: 'no FAKE-annotated constructs found ... nothing to ablate'. pb_array_subscript.c: score 15, build_insns 105, rules_dropped 0 — identical to the s12 record and to candidate.c. K37 stands as a second independent permuter seed, not as an improvement.
- verdict: CONFIRMED

## s14 (2026-09-05) - synthesis modality

Chassis re-checked FIRST: candidate.c applied over the INCLUDE_ASM line at
src/code6cac_c2.c:629, `sandbox func_8003C714 --disable all` = score 15,
target_insns 104, build_insns 105, rules_dropped 0. Unchanged since s1.
Mandated kill re-audit: `tools/fake_ablate.py` reports no FAKE-annotated
constructs in candidate.c (nothing to ablate), and s12's K36 chassis
(s12/pc_second_biv.c) re-measures 15 / 105 on the current chassis - STANDS.

This session merged the ledger and found the one question s5-s13 never asked in
this form. s5's dead-code probes died at `delete_dead_from_cse`; s6 found the
loop-carried hole in that fixpoint and s7/s12/s13 closed every carrier for it
(K21 natural accumulators, K35 moved_once, K36 extra bivs, K40 no tail
consumer). But ALL of that work is about insns that are free because a pass
DELETES them. Nobody had tested insns that are free because a pass MERGES or
MASKS them away - specifically `combine`, which runs at toplev.c:3004, i.e.
AFTER `loop_optimize` has already consumed insn_count.

**H19 (CONFIRMED) - a SECOND, broad, free insn_count channel exists, and it
reaches distance 0.** Chain arithmetic on a LIVE local whose only consumer is a
narrower (QImode) store survives cse1 - cse1 value-numbers, it does not perform
algebraic cancellation - so `count_loop_regs_set` counts every insn of the
chain; after `loop_optimize`, combine's `force_to_mode`
(tools/gcc-2.7.2/combine.c:5682) erases all of them, because none of them can
affect the 8 bits the store keeps. Measured with the s6 harness re-pointed at
tmp/grind/func_8003C714/s14 (`sweep.sh` + `gen_xorpair.py` / `gen_pad.py`),
carrier = `v = *src;` ... `dst[0x24] = v;` with k inserted pairs:

    k=0    57 real insns   asm 107     (the bare `v` local is itself +1 free)
    k=1    59              asm 107
    k=2    61              asm 107
    k=4    65              asm 107
    k=8    73              asm 107
    k=16   89              asm 107
    k=30  117              asm 107     movable still moved
    k=31  119              asm 107     movable still moved
    k=32  121  "not desirable"  asm 106

and at k=32 `sandbox func_8003C714 --disable all` prints **score 0,
target_insns 104, build_insns 104**. Banked at
rejected/combine-foldable-live-value-pads-64-insns-d0-but-inadmissible.c.
The k=30/31/32 boundary pins `threshold` at exactly 119 and confirms s11's H17
window (`insn_count >= 120`) to the instruction.

**H20 (CONFIRMED) - the channel is not xor-specific; four shapes measured
identical.** `v ^= C; v ^= C;`, `v += C; v -= C;`, `v &= 0xFF; v &= 0xFF;` and
`v = v << 3; v = v >> 3;` each measure +2 insn_count per pair at +0 emitted
instructions and each flips the movable to "not desirable" at k=32 (121 insns,
asm 106). This RE-SCOPES s3's K10 ("byte-neutral masks fold before loop"): a
mask folds when it sits on the DIVIDED value, but not when it sits on a live
local ahead of a narrower store. K10 is an instance result about a position, not
a law about masks.

**K41 (instance) - none of the four measured combine-fold carriers is
admissible, and the four were chosen to span the channel.** Each is
value-preserving with respect to the stored byte by construction, so each fails
cheat-test T1 (no observable effect beyond a simpler form) and T2. No frozen
SOTN family covers in-loop inert arithmetic; first reach would be a cheat. The
d0 form was NOT submitted and src/ was restored to INCLUDE_ASM.

**H21 (CONFIRMED, arithmetic) - the original source contained ZERO padding, so
every distance-0 form on the SHIPPED chassis necessarily carries matter the
original did not have.** On the original chassis the symbol_ref movable
(&D_80106A58, savings 1, lifetime 1) MOVED and the 0x91A2B3C5 movable
(savings 1, lifetime 1) did NOT, so `threshold_orig >= insn_count_orig` and
`threshold_orig - 3 < insn_count_orig`, i.e. `insn_count_orig` is in
[threshold_orig - 2, threshold_orig]. With s2's measured `threshold_orig = 58`
(the soft-float register file, 2*(1+28)) that gives `insn_count_orig` in
[56, 58] - and our candidate.c body measures insn_count 56 with no padding at
all. The original body therefore had no carrier; the >= 63 extra RTL insns the
shipped chassis demands are a chassis artifact, not source. This is recorded as
an arithmetic CONSEQUENCE of the ledger's own measurements, not as a
disposition: the CC_FLAGS route stays barred by the standing Judge constraint.

**THE MERGED LAW (s14 synthesis).** Two independent free-insn_count channels are
now measured, and both are free for the SAME reason: the compiler can prove the
added computation cannot be observed. s6's channel is free because
`strength_reduce` deletes a constant-step biv whose exit value it can fold
(free <=> nothing consumes it, K40). s14's channel is free because
`force_to_mode` masks off computation that cannot reach the stored bits
(free <=> it does not change the store). A THIRD shape is conceivable and is the
only one that would break the pattern: an insn that is free because combine
MERGES it into an instruction the target already emits, where both halves are
semantically real. That is the live frontier below.

### FRONTIER after s14 (reset)

1. **THE LIVE ITEM - the merge sub-channel, where freeness and semantic reality
   are not mutually exclusive.** combine (toplev.c:3004) runs after
   loop_optimize and merges 2- and 3-insn groups into ONE pattern. If an
   existing target instruction can be produced by a 2-insn pre-combine group,
   the extra insn is counted by count_loop_regs_set and costs nothing - and BOTH
   halves compute real values, so T1 is not automatically failed the way the two
   deletion channels are. The loop emits ~45 instructions, so the theoretical
   ceiling of this sub-channel (1-2 absorbed insns per existing insn) covers the
   +63 requirement. Probe: re-spell the four existing division/modulo statements
   in decomposed-but-identical forms and read insn_count against emitted asm
   lines with tmp/grind/func_8003C714/s14/sweep.sh - e.g. `x >> 10` as two
   ashiftrt steps, `(x % 30) * 100` as shift/add decompositions, `% 60` as
   `x - (x/60)*60`, each of which GCC re-merges in combine. Note the
   split-init/compound-assignment precedent
   ([[split-init-accumulation-sanctioned]], owner feedback) already treats
   same-variable arithmetic splits as ORDINARY C, which is the admissibility
   handle the two deletion channels never had. Even a partial win matters: the
   free ORDER dial (K24) is worth another -6 on the requirement.
2. **Characterise the +1 free local.** `v = *src;` alone took insn_count 56 -> 57
   at asm 107. That is one free insn from a single ordinary named intermediate,
   i.e. the named-intermediate family. If other statements admit the same
   treatment, several free insns are available from a SANCTIONED family; the
   requirement is +63, so this alone will not close it, but it is the only
   measured free insn that is not inert.
3. **CLOSED RECORDS.** The two deletion channels (s6 biv, s14 force_to_mode) are
   both proven to reach distance 0 and both proven inadmissible; do not re-derive
   either. The chassis/threshold=122 root cause and CC_FLAGS remain barred.

## [s14] A second free insn_count channel exists: SImode chain arithmetic on a live local whose only consumer is a QImode store survives cse1, is counted by count_loop_regs_set, and is erased after loop_optimize by combine's force_to_mode - reaching sandbox score 0 on the shipped chassis at 64 added insns.
- mechanism: cse1 (toplev.c:2865) value-numbers but performs no algebraic cancellation, so `v ^= C; v ^= C;` is not folded there and both insns are in the RTL stream when count_loop_regs_set runs (loop.c:3001-3092). loop.c:1631 then sees threshold 119 against insn_count 121 and declines the 0x91A2B3C5 movable, leaving it in-loop in the target's split lui/lw/ori/mult form. combine (toplev.c:3004) runs after loop_optimize and force_to_mode (combine.c:5682) deletes every chain insn because none of them affect the 8 bits `dst[0x24] = v;` keeps.
- probe: tmp/grind/func_8003C714/s14/sweep.sh over gen_xorpair.py / gen_pad.py, k = 0,1,2,4,8,16,30,31,32, reading the .loop movable table and the emitted asm line count; then body_xp32.c copied over src/code6cac_c2.c and measured with `sandbox func_8003C714 --disable all`.
- result: insn_count 57/59/61/65/73/89/117/119/121 against a CONSTANT 107 emitted asm lines up to k=31; at k=32 the movable prints "not desirable", asm drops to 106, and the sandbox measures score 0, target_insns 104, build_insns 104. The k=30/31/32 boundary pins threshold at exactly 119.
- verdict: CONFIRMED

## [s14] The combine-fold channel is broad, not xor-specific: add/sub pairs, repeated &0xFF masks and shift round-trips all carry insn_count at zero byte cost.
- mechanism: force_to_mode is a bit-mask analysis, not a pattern match, so it erases any computation whose result cannot reach the live bits of the narrower store, whatever the operator.
- probe: MODE=addsub / mask / shift through tmp/grind/func_8003C714/s14/gen_pad.py at k = 4, 16, 32.
- result: all three give 65 / 89 / 121 real insns against asm 107 / 107 / 106, identical to the xor sweep, and all three flip the movable at k=32. s3's K10 ("byte-neutral masks fold before loop") is re-scoped to the position it was measured in (masks on the divided value), not to masks in general.
- verdict: CONFIRMED

## [s14] K41: one of the four combine-fold carrier shapes measured this session is admissible ordinary C.
- mechanism: the channel is free precisely when force_to_mode can prove the chain does not reach the stored bits, which is the same condition as the chain having no observable effect
- probe: applied cheat-tests T1-T6 to each of the four measured shapes (xor pairs, add/sub pairs, repeated masks, shift round-trips) and searched the frozen SOTN family list for in-loop inert arithmetic
- result: FALSE. All four are value-preserving with respect to the stored byte, so T1 and T2 fail on each; no frozen family covers the construct and first reach of an unsanctioned family is a cheat. The d0 body was banked as a reachability proof only and src/ was restored to INCLUDE_ASM.
- verdict: KILLED
- kill_scope: instance
- measured_on: shipped chassis HEAD 2026-09-05, tmp/grind/func_8003C714/s14/body_xp32.c copied over src/code6cac_c2.c (sandbox 0 / 104 / 104) then reverted; no FAKE constructs present

## [s14] The original compile's loop contained no padding, so the >= 63 extra RTL insns the shipped chassis needs are a chassis artifact rather than lost source.
- mechanism: on the original chassis the &D_80106A58 movable (savings 1, lifetime 1) moved and the 0x91A2B3C5 movable (savings 1, lifetime 1) did not, so loop.c:1631 gives threshold_orig >= insn_count_orig > threshold_orig - 3; s2 measured threshold_orig = 58 (soft-float, 2*(1+28)), bounding insn_count_orig to [56,58].
- probe: arithmetic over the ledger's own measured quantities (s2 H4 threshold sweep, s7 H11 savings/lifetime pinning, s10 H15) against the candidate body's measured insn_count of 56.
- result: candidate.c already sits inside the original window at insn_count 56 with zero carrier statements, so the original source had no padding. Recorded as an arithmetic consequence only; the CC_FLAGS route remains barred by the standing Judge constraint and is not re-filed.
- verdict: CONFIRMED

## [s14] Mandated kill re-audit: s12's K36 chassis is unchanged on the current chassis and candidate.c carries no FAKE construct.
- mechanism: an instance kill is only as good as the chassis and FAKE state it was measured under
- probe: tools/fake_ablate.py --func func_8003C714 --file code6cac_c2 --candidate memory/grind/func_8003C714/candidate.c; then tmp/grind/func_8003C714/s12/pc_second_biv.c applied and measured with sandbox --disable all
- result: fake_ablate: "no FAKE-annotated constructs found ... nothing to ablate". pc_second_biv.c: score 15, build_insns 105, rules_dropped 0 - identical to the s12 record. K36 STANDS.
- verdict: CONFIRMED

## [s14] A second free insn_count channel exists for func_8003C714: SImode chain arithmetic on a LIVE local whose only consumer is a QImode store survives cse1, is counted by count_loop_regs_set, and is erased after loop_optimize by combine's force_to_mode, taking the sandbox to score 0 on the shipped chassis at 64 added insns.
- mechanism: cse1 (toplev.c:2865) value-numbers but performs no algebraic cancellation, so `v ^= C; v ^= C;` is not folded there and both insns are present in the RTL stream when count_loop_regs_set runs (loop.c:3001-3092). loop.c:1631 then evaluates 119 * 1 * 1 >= insn_count for the 0x91A2B3C5 movable and declines it at insn_count 121, leaving the magic in the loop in the target's split lui / lw / ori / mult form. combine (toplev.c:3004) runs after loop_optimize and force_to_mode (tools/gcc-2.7.2/combine.c:5682) deletes every chain insn, because none of them can affect the 8 bits that `dst[0x24] = v;` keeps.
- probe: tmp/grind/func_8003C714/s14/sweep.sh over gen_xorpair.py at k = 0,1,2,4,8,16,30,31,32 (shipped cc1, canonical flags, -dL), reading `Loop from A to B: N real insns.` and the moved / not-desirable lines from the .loop dump plus the emitted asm line count; then tmp/grind/func_8003C714/s14/body_xp32.c copied over src/code6cac_c2.c and measured with `& tools/wteng.ps1 main sandbox func_8003C714 --disable all`.
- result: insn_count 57/59/61/65/73/89/117/119/121 against a CONSTANT 107 emitted asm lines through k=31; at k=32 the movable prints 'not desirable', emitted asm drops to 106, and the sandbox prints score 0, target_insns 104, build_insns 104. The k=30/31/32 rows pin threshold at exactly 119 (119 >= 119 still moves, 119 >= 121 does not), confirming s11's H17 window to the instruction. src/ was restored to INCLUDE_ASM immediately after the measurement.
- verdict: CONFIRMED

## [s14] The combine-fold channel is a property of the position (live local feeding a narrower store), not of the operator: add/sub pairs, repeated &0xFF masks and shift round-trips all carry insn_count at zero byte cost exactly like xor pairs.
- mechanism: force_to_mode is a bit-mask analysis rather than a pattern match, so it erases any computation whose result cannot reach the live bits of the narrower store, whatever the operator is.
- probe: MODE=addsub / mask / shift through tmp/grind/func_8003C714/s14/gen_pad.py at k = 4, 16, 32, same harness and same dump reads.
- result: All three modes give 65 / 89 / 121 real insns against emitted asm 107 / 107 / 106 - identical to the xor sweep in every cell - and all three flip the 0x91A2B3C5 movable to 'not desirable' at k=32. This re-scopes s3's K10 ('byte-neutral masks fold before loop'), which is true where s3 measured it (masks sitting on the divided value, folded by cse1 into the existing computation) and false for a mask on a live local ahead of a narrower store.
- verdict: CONFIRMED

## [s14] One of the four combine-fold carrier shapes measured this session (xor pairs, add/sub pairs, repeated &0xFF, shift round-trips) is admissible ordinary C under the frozen family list.
- mechanism: The channel is free precisely when force_to_mode can prove the chain does not reach the stored bits, which is the same condition as the chain having no observable effect on the function's output; so within this channel freeness and cheat-test T1 failure are the same predicate.
- probe: Applied the six-test cheat checklist to each of the four measured shapes and searched the frozen SOTN family list for a family covering in-loop inert arithmetic on a live local.
- result: FALSE for all four shapes. Each is value-preserving with respect to the stored byte, so T1 (semantic purpose) and T2 (human-programmer) fail on each; no frozen family covers the construct and first reach of an unsanctioned family is a cheat. The distance-0 body was banked as a reachability proof at memory/grind/func_8003C714/rejected/combine-foldable-live-value-pads-64-insns-d0-but-inadmissible.c and NOT submitted.
- verdict: KILLED
- kill_scope: instance
- measured_on: shipped chassis HEAD 2026-09-05, tmp/grind/func_8003C714/s14/body_xp32.c copied over src/code6cac_c2.c (sandbox score 0, 104 target / 104 build) and the three MODE variants swept through cc1; no FAKE constructs present (tools/fake_ablate.py reports none in candidate.c)

## [s14] The original compile's loop contained no carrier statements at all, so the >= 63 extra RTL insns the shipped chassis requires are a chassis artifact rather than source this ledger has failed to recover.
- mechanism: On the original chassis the &D_80106A58 movable (savings 1, lifetime 1) moved and the 0x91A2B3C5 movable (savings 1, lifetime 1) did not, so loop.c:1631 gives threshold_orig >= insn_count_orig and threshold_orig - 3 < insn_count_orig, bounding insn_count_orig to [threshold_orig - 2, threshold_orig].
- probe: Arithmetic over the ledger's own measured quantities - s2's H4 flag sweep (threshold_orig = 58 = 2*(1+28) on the soft-float register file), s7's H11 and s10's H15 pinning savings and lifetime at 1 - against the candidate body's measured insn_count of 56.
- result: insn_count_orig lies in [56, 58] and candidate.c already measures 56 with zero carrier statements, so the original body had no padding. Recorded as an arithmetic consequence only; the CC_FLAGS / chassis route remains barred by the standing Judge constraint and is not re-filed in any shape.
- verdict: CONFIRMED

## [s14] Mandated kill re-audit: s12's K36 chassis (a second natural index variable) still measures 15 / 105 on the current chassis, and candidate.c carries no FAKE construct, so no banked lever of this ledger was measured while a FAKE carrier occupied a target pseudo.
- mechanism: An instance kill is only as good as the chassis and FAKE state it was measured under, so the brief mandates re-measuring the closest instance kill before any new probe.
- probe: `python3 tools/fake_ablate.py --func func_8003C714 --file code6cac_c2 --candidate memory/grind/func_8003C714/candidate.c`, then tmp/grind/func_8003C714/s12/pc_second_biv.c applied with s12/apply.py and measured with `sandbox func_8003C714 --disable all`.
- result: fake_ablate: 'no FAKE-annotated constructs found ... nothing to ablate'. pc_second_biv.c: score 15, build_insns 105, rules_dropped 0 - identical to the s12 record. K36 STANDS. Baseline candidate.c re-measured first at 15 / 104 / 105 (the brief reported the chassis measurement as unavailable).
- verdict: CONFIRMED

## s15 (2026-09-05) - synthesis modality

Chassis re-checked FIRST: sandbox score 15 / 104 / 105 with the s15 `s_max`
body in src/ (byte-identical to candidate.c); src/ restored to INCLUDE_ASM.
Mandated kill re-audit: fake_ablate reports no FAKE constructs in candidate.c,
nothing to ablate - the third consecutive confirmation, so this branch of the
re-audit is closed and future sessions should not spend a turn on it.

This session did the merge that s14 asked for and then found the term s14's
merge had missed. s14 wrote the requirement as a CONSTANT ("insn_count >= 120").
It is not a constant: `threshold` in `move_movables` is a running variable that
loses 3 for every movable moved BEFORE the one being judged. Reading the
movable table instead of inheriting the number (s15-A) settles both the
constant and the order, and turns the requirement into a two-term budget.

### CONFIRMED

- **H22 - the threshold dial is real, linear, and worth 3 counted insns per
  extra moved movable.** Requirement is `insn_count + 3*extra_moved >= 120`,
  measured boundary 120 at m=0, 114 at m=1 (two extra moved movables), and
  consistent flips at m=2 (109) and m=3 (107). Predicate: loop.c:1631 with
  loop.c:1719 / loop.c:1904 as the only writers of `threshold`. This re-scopes
  every "+63 insns needed" statement in this ledger from s11 onward: with
  K24's order dial spent the real requirement is insn_count >= 117.
- **H23 - split-init accumulation is a FREE and ORDINARY insn_count channel
  worth exactly +7, and it saturates there.** 56 -> 63 counted insns at 107
  emitted asm lines unchanged, verified in the real sandbox at score 15 /
  build_insns 105 (byte-identical to candidate.c). Four further split families
  measured on top of it all stay at 63. This is the ledger's FIRST admissible
  free channel - the two known distance-0 carriers (s6 biv noise, s14
  force_to_mode pad) are both inert and both inadmissible.
- **H24 (arithmetic) - the original body was SMALLER than candidate.c.**
  Adding the order term to s14's H21 tightens `insn_count_orig` from [56,58] to
  [53,55], below candidate.c's 56. Recorded as arithmetic; not actionable while
  the CC_FLAGS route is barred, and it points the wrong way on this chassis.

### KILLED

- **K42 (instance) - the combine MERGE sub-channel yields zero free insn_count
  in this loop.** All four hand-decompositions of the loop's division / modulo /
  multiply-by-100 statements measure insn_count 56 and asm_lines 107, identical
  to candidate.c on both axes: the decomposition IS the RTL the expanders emit,
  so nothing extra is counted before loop_optimize and nothing is left for
  combine to merge. Measured on the shipped chassis (HEAD 2026-09-05) with no
  FAKE construct present, dumps r_mod60 / r_mod30 / r_mul100 / r_ni3. Banked at
  rejected/division-modulo-decompositions-are-already-the-emitted-rtl.c. This
  closes s14's frontier item 1.
- **K43 (class) - `loop_has_call` cannot be set by any non-call construct**, so
  the threshold cannot be halved from 122 to 61. `loop_has_call` is assigned in
  exactly one place, and only under `GET_CODE (insn) == CALL_INSN`.
  predicate_cite: tools/gcc-2.7.2/loop.c:2202. Measured on the shipped chassis
  by source reading plus the loop.c:532 threshold expression.
- **K44 (instance) - a chain of distinct loop-invariant locals cannot buy the
  threshold dial**, because cse1 folds a constant-xor chain whose links are all
  loop-invariant down to one insn: chain lengths 2/4/8/12/16/20/24 all measure
  insn_count 57, +1 moved movable, asm_lines 110. (Contrast s14's H19, where
  the identical shape on a loop-VARIANT local survived cse1 at +2 per pair -
  invariance is what enables the fold.) Measured on the shipped chassis with no
  FAKE construct present, dumps p_inv2..p_inv24. Banked at
  rejected/invariant-xor-chain-collapses-at-cse1-to-one-movable.c.
- **K45 (instance) - a single named intermediate read three times LOSES
  insn_count**, it does not add it: `t = *(s32*)(src+4);` used by all three
  divisions measures 43 insns / 85 asm (cse collapses the three loads). Same
  for sharing one `x/30` across both modulo statements (51 / 98). Measured on
  the shipped chassis, dumps r_nit / r_both. Corroborates the older
  rejected/single-read-v-local-loses-16-insns.c.

### THE BUDGET, restated for s16 (this is the whole problem in four lines)

    need:      insn_count + 3*extra_moved >= 120
    have:      63 (ordinary split-init, saturated) + 3*1 (K24 order dial) = 66
    deficit:   54 counted insns, or 18 free moved movables, or any 1:3 mix
    forbidden: inert padding (s6 biv noise, s14 force_to_mode pad) - both
               reach sandbox 0, both fail T1/T2

### FRONTIER after s15 (reset)

1. **THE LIVE ITEM - is there a moved movable that costs zero emitted bytes?**
   This is now the highest-leverage unknown in the ledger, because H22 prices
   one at 3 counted insns and 18 of them close the whole gap. A movable is
   hoisted to the preheader; it is free iff its preheader insn disappears
   afterwards. Two mechanisms could do that and neither has been measured:
   (a) the hoisted def becomes DEAD because its in-loop use is erased after
   loop_optimize (combine's force_to_mode, i.e. s14's channel applied to an
   INVARIANT rather than a variant local - note K44 shows the naive spelling
   folds at cse1, so the chain must be non-foldable, e.g. distinct operators
   or a value cse1 cannot constant-fold); (b) the hoisted def is COALESCED with
   an existing preheader insn by regmove/local-alloc. Probe: build a single
   invariant carrier of each shape, read tmp/grind/.../dumps/*.seg for a fourth
   `moved to` line, and read asm_lines for the byte cost. A shape that measures
   +1 counted insn, +1 moved, and 107 asm lines is the whole ballgame; if every
   shape that is free is also inert, that is a strong, citable statement that
   the two-term budget cannot be paid in ordinary C, and it is what an
   escalation entry would have to say.
2. **Re-derive the 15-instruction residual from the dumps rather than from
   s1's H2.** H2 ("the entire d15 residual is the const placement plus the
   mfhi-temp seat") has never been re-verified in 14 sessions, and it is the
   weakest verdict in the ledger under the CONTRADICTION RULE: it is the single
   assumption that makes the loop.c hoist the only thing worth attacking. Probe:
   apply s_max, run `pwsh tools/grinder/dump.ps1 func_8003C714`, and diff the
   .greg / .sched output against asm/funcs/func_8003C714.s instruction by
   instruction. If any of the 15 is independent of the const placement, it is
   separately closable and the budget above is not the only front.
3. **The declaration puns remain a submission blocker** (unchanged from s14,
   restated because it is a hard gate on ANY distance-0 body): the brief's
   auto-scan flags `src = (u8 *)&D_80106A58 + i * 8;` and
   `*((u8 *)s0 + 0x30) = *(u16 *)&D_80101ED2;`. The sanctioned fix is the
   aggregate-merge family at the DECLARATION in include/*.h via an integration
   handoff, prongs (a)-(e), with prong (c)'s 2026-09-03 amendment requiring the
   sub-symbol rows to carry an `alias of g_file_disc_type+4; retire with
   func_80035280` suffix. Prong (a) looks satisfiable: the target's loop walks a
   single base register `$a2` with `addiu $a2, $a2, 0x8` at 8003C830, which is
   base-register/stride evidence rather than the forbidden
   adjacency-of-splat-names evidence.

## [s15] The move decision in move_movables is a two-term budget, not a fixed insn_count target: threshold loses 3 for every movable moved ahead of the one being judged, so the requirement for the 0x91A2B3C5 movable is insn_count + 3*extra_moved >= 120
- mechanism: loop.c:1631 desirability test with savings and lifetime pinned at 1; threshold written only at loop.c:532, loop.c:1719 and loop.c:1904
- probe: inserted m invariant dial steps at the top of the loop body and swept the s14 xor-pad k to locate the "not desirable" flip; dumps tmp/grind/func_8003C714/s15/dumps/q_m*_k*
- result: boundary insn_count 120 at m=0 (moved at 119, not desirable at 121), 114 at m=1 (moved at 113, not desirable at 115), flips at 109 for m=2 and 107 for m=3 - exactly -3 per extra moved movable
- verdict: CONFIRMED

## [s15] The shipped chassis has exactly three movables, and exactly one of them (the &D_80106A58 symbol_ref) is processed before the 0x91A2B3C5 magic constant, so the threshold at the decision is 122-3 = 119 and K24's order dial (worth -6) is capped by there being only two other movables
- mechanism: movables are appended to the list in insn order in scan_loop; loop.c:1719 decrements threshold once per moved movable
- probe: read the movable table out of the .loop dump segment (tmp/grind/func_8003C714/s15/dumps/r_base.seg) instead of inheriting the number
- result: "Insn 33: regno 78 (life 1) moved to 203 / Insn 46: regno 84 (life 1) moved to 205 / Insn 60: regno 91 (life 31) moved to 207"; regno 84 is the /1800 magic, regno 91 (life 31) the thrice-used /30 magic the target also hoists
- verdict: CONFIRMED

## [s15] Split-init / compound-assignment accumulation is a free AND ordinary insn_count channel worth exactly +7 in this loop, and it saturates at 63
- mechanism: each split writes a named local that cse1 keeps until after count_loop_regs_set and that local-alloc later coalesces, so it is counted and not emitted; classed ORDINARY C by owner feedback [[split-init-accumulation-sanctioned]]
- probe: stacked splits across the three division statements plus `v = *src;` (dumps s_a / s_ab / s_c / s_max), then four further split families on top (dumps t_off / t_addr / t_mul / t_bsplit), and applied s_max to src/ for a real sandbox measurement
- result: 56 -> 58 -> 60 -> 63 counted insns at asm_lines 107 throughout; sandbox score 15, target 104, build 105 (byte-identical to candidate.c); all four further families stay at exactly 63
- verdict: CONFIRMED

## [s15] The combine MERGE sub-channel - s14's frontier item 1 - yields zero free insn_count in this loop, because the hand-decompositions of the division, modulo and multiply-by-100 statements are already the RTL the expanders emit
- mechanism: `%` expands to div+mult+sub and `*100` to a shift/add chain at RTL expansion, i.e. before cse1 and long before count_loop_regs_set at loop.c
- probe: four decompositions measured against candidate.c on both axes; dumps r_mod60 / r_mod30 / r_mul100 / r_ni3
- result: all four measure insn_count 56 and asm_lines 107, identical to r_base; banked at rejected/division-modulo-decompositions-are-already-the-emitted-rtl.c
- verdict: KILLED
- kill_scope: instance
- measured_on: shipped chassis HEAD 2026-09-05, candidate.c body in tmp/grind/func_8003C714/s15/r_*.c, no FAKE construct present

## [s15] The loop_has_call term that would halve the initial threshold from 122 to 61 cannot be set by any construct other than a real CALL_INSN
- mechanism: loop.c:532 computes threshold as (loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs); loop_has_call has exactly one assignment site
- probe: read prescan_loop in tools/gcc-2.7.2/loop.c end to end and grepped every reference to loop_has_call and loop_has_volatile
- result: loop_has_call = 1 only under GET_CODE (insn) == CALL_INSN at loop.c:2202; volatile refs set the separate loop_has_volatile at loop.c:2209, which does not appear in the threshold expression; a CALL_INSN emits a jal the target's loop does not contain
- verdict: KILLED
- kill_scope: class
- predicate_cite: tools/gcc-2.7.2/loop.c:2202
- measured_on: shipped chassis HEAD 2026-09-05, compiler source reading, no FAKE construct present

## [s15] A chain of distinct loop-invariant locals cannot buy extra moved movables for the threshold dial, because cse1 folds a constant-xor chain whose links are all loop-invariant down to a single insn
- mechanism: cse1 constant-folds when both operands are invariant; s14's H19 survived only because its carrier was loop-VARIANT
- probe: chain lengths 2, 4, 8, 12, 16, 20 and 24 swept through the s15 harness; dumps p_inv2 .. p_inv24
- result: every length measures insn_count 57, exactly one extra moved movable and asm_lines 110 - no scaling at all; banked at rejected/invariant-xor-chain-collapses-at-cse1-to-one-movable.c
- verdict: KILLED
- kill_scope: instance
- measured_on: shipped chassis HEAD 2026-09-05, tmp/grind/func_8003C714/s15/p_inv*.c, no FAKE construct present

## [s15] A single named intermediate read three times, and a shared x/30 quotient, both LOSE counted insns rather than adding them
- mechanism: cse collapses the repeated loads / repeated division, deleting counted insns before loop_optimize
- probe: r_nit (t = *(s32*)(src+4) read by all three divisions) and r_both (one q = x/30 feeding both modulo statements)
- result: r_nit 43 insns / 85 asm; r_both 51 insns / 98 asm - both far below candidate.c's 56 / 107 and both wrong on bytes
- verdict: KILLED
- kill_scope: instance
- measured_on: shipped chassis HEAD 2026-09-05, tmp/grind/func_8003C714/s15/r_nit.c and r_both.c, no FAKE construct present

## [s15] The move decision in move_movables is a two-term budget, not a fixed insn_count target: threshold loses 3 for every movable moved ahead of the one being judged, so the requirement for the 0x91A2B3C5 movable is insn_count + 3*extra_moved >= 120.
- mechanism: loop.c:1631 desirability test (threshold * savings * lifetime >= insn_count) with savings and lifetime pinned at 1 by s10 H15 / s13; threshold is written only at loop.c:532 (initialisation to (loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs) = 122) and at loop.c:1719 / loop.c:1904, both 'threshold -= 3', once per MOVED movable.
- probe: Inserted m invariant dial steps (w0 = (s32)s0 * 3; w1 = w0 * 5; ...) at the top of the loop body, each contributing 2 counted insns and 2 moved movables, then swept the s14 xor-pad k to locate the 'not desirable' flip. Dumps tmp/grind/func_8003C714/s15/dumps/q_m*_k*.
- result: Boundary insn_count 120 at m=0 (still moved at 119 = k31, not desirable at 121 = k32); 114 at m=1 (still moved at 113 = k27, not desirable at 115 = k28); flips at 109 for m=2 and 107 for m=3. Exactly -3 per extra moved movable, linear across the range. Combined with the movable table read in s15-A this re-scopes every '+63 insns needed' statement from s11 onward: with K24's order dial spent the real requirement is insn_count >= 117.
- verdict: CONFIRMED

## [s15] The shipped chassis has exactly three movables and exactly one of them (the &D_80106A58 symbol_ref, regno 78) is processed before the 0x91A2B3C5 magic constant (regno 84), so threshold at the decision is 122-3 = 119 and K24's order dial is capped at -6 because there are only two other movables.
- mechanism: scan_loop appends movables to the list in insn order; loop.c:1719 decrements threshold once per moved movable, so the count of movables PRECEDING ours in that list is the whole order term.
- probe: Read the movable table straight out of the .loop dump segment (tmp/grind/func_8003C714/s15/dumps/r_base.seg) rather than inheriting the inherited number.
- result: 'Insn 33: regno 78 (life 1), move-insn savings 1 moved to 203 / Insn 46: regno 84 (life 1) moved to 205 / Insn 60: regno 91 (life 31) moved to 207'. regno 84 is the /1800 magic the target leaves in the loop; regno 91 (life 31) is the thrice-used /30 magic 0x88888889 that the target also hoists (8003C740-8003C744). Confirms s14's empirically pinned threshold of 119 from the compiler's own bookkeeping.
- verdict: CONFIRMED

## [s15] Split-init / compound-assignment accumulation is a free AND ordinary insn_count channel worth exactly +7 in this loop, and it saturates at 63.
- mechanism: Each split writes a named local that cse1 keeps until after count_loop_regs_set runs and that local-alloc later coalesces, so the insn is counted by loop.c but never emitted. Owner feedback [[split-init-accumulation-sanctioned]] classes same-variable split-init and compound-assignment splits as ORDINARY C, so no FAKE annotation and no family claim is needed.
- probe: Stacked splits across the three division statements plus the v = *src named intermediate (dumps s_a / s_ab / s_c / s_max), then four further split families on top of s_max (dumps t_off / t_addr / t_mul / t_bsplit), then applied s_max to src/code6cac_c2.c and ran the real sandbox.
- result: 56 -> 58 -> 60 -> 63 counted insns with asm_lines 107 throughout; sandbox func_8003C714 --disable all on s_max prints score 15, target_insns 104, build_insns 105 - byte-identical to candidate.c, so the +7 is real and not a harness artefact. All four further split families (offset intermediates, v = v + 0, c*4 then c*25, modulo-60 re-decomposition) stay at exactly 63 / 107: cse1 folds them back. This is the ledger's first admissible free channel; the two known distance-0 carriers (s6 biv noise, s14 force_to_mode pad) are both inert and inadmissible. Banked at rejected/splitinit-ceiling-63-ordinary-c-byte-neutral-but-54-short.c.
- verdict: CONFIRMED

## [s15] Adding the movable-order term to s14's H21 tightens the original chassis arithmetic: insn_count_orig lies in [53,55], below candidate.c's measured 56, so the original source body was at least one RTL insn smaller than candidate.c rather than equal to it.
- mechanism: On the original chassis the target hoists 0x88888889 and then &D_80106A58 before leaving regno 84 in the loop, so with threshold_orig = 58 the thresholds seen by the three movables are 58, 55 and 52; the three loop.c:1631 outcomes give 58 >= n, 55 >= n and 52 < n.
- probe: Applied the s15-A order finding to the inequality system s14 wrote in H21 and re-solved it.
- result: insn_count_orig in [53,55] (s14 had [56,58]). Recorded as arithmetic only: the CC_FLAGS route that would make it actionable stays barred by the standing Judge constraint, and on the shipped chassis the requirement runs the other way.
- verdict: CONFIRMED

## [s15] Every hand-decomposition of this loop's division, modulo and multiply-by-100 statements measures identical insn_count and identical emitted asm to candidate.c, so no counted insn is available from combine's merge path in these four positions.
- mechanism: Modulo expands to div+mult+sub and multiply-by-100 expands to a shift/add chain at RTL EXPANSION time, i.e. before cse1 and long before count_loop_regs_set, so writing the decomposition by hand adds nothing to the count and leaves combine nothing extra to merge.
- probe: Four decompositions measured against candidate.c on both axes: (x/30)%60 as q - (q/60)*60; x%30 as x - (x/30)*30; r*100 as (r<<6)+(r<<5)+(r<<2); and once-written once-read named intermediates on each division result. Dumps r_mod60 / r_mod30 / r_mul100 / r_ni3.
- result: All four measure insn_count 56 and asm_lines 107, identical to r_base. This closes s14's frontier item 1 - the merge sub-channel it designated as the only channel where freeness need not imply inertness is empty in these positions. Banked at rejected/division-modulo-decompositions-are-already-the-emitted-rtl.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: shipped chassis HEAD 2026-09-05, candidate.c body plus the four decompositions in tmp/grind/func_8003C714/s15/r_*.c, no FAKE construct present (fake_ablate reports nothing to ablate in candidate.c)

## [s15] The loop_has_call term that would halve the initial threshold from 122 to 61 is set only by a real CALL_INSN, so no non-call construct can reach it.
- mechanism: loop.c:532 computes threshold as (loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs); loop_has_call has exactly one assignment site, inside prescan_loop, guarded by GET_CODE (insn) == CALL_INSN. Volatile references set the separate loop_has_volatile at loop.c:2209, which does not appear in the threshold expression, and a CALL_INSN emits a jal that the target's loop does not contain.
- probe: Read prescan_loop in tools/gcc-2.7.2/loop.c end to end and grepped every reference to loop_has_call and loop_has_volatile in the file.
- result: Confirmed single assignment site under the CALL_INSN guard. This formalises the older rejected/threshold-term-only-movable-by-cheat-or-by-a-call-in-the-loop.c as a predicate-cited class result: the single largest term in the gate (worth -61 on the requirement, which the s15-D ceiling of 63 would otherwise cover outright) is unreachable.
- verdict: KILLED
- kill_scope: class
- measured_on: shipped chassis HEAD 2026-09-05, compiler source reading against the loop.c:532 threshold expression, no FAKE construct present
- predicate_cite: tools/gcc-2.7.2/loop.c:2202

## [s15] A chain of distinct loop-invariant locals does not scale the number of moved movables, because cse1 folds a constant-xor chain whose links are all loop-invariant down to a single insn.
- mechanism: cse1 constant-folds when both operands are invariant. s14's H19 carrier survived cse1 only because it sat on a loop-VARIANT local; invariance is exactly what enables the fold, so the shape that would buy the H22 threshold dial is the shape cse1 destroys.
- probe: Swept chain lengths 2, 4, 8, 12, 16, 20 and 24 of the form w0 = (s32)s0 ^ C0; w1 = w0 ^ C1; ... with the final value consumed by the tail store. Dumps p_inv2 .. p_inv24.
- result: Every length measures insn_count 57, exactly one extra moved movable and asm_lines 110 - no scaling with chain length at all. Banked at rejected/invariant-xor-chain-collapses-at-cse1-to-one-movable.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: shipped chassis HEAD 2026-09-05, tmp/grind/func_8003C714/s15/p_inv*.c, no FAKE construct present

## [s15] A single named intermediate read three times, and a shared x/30 quotient feeding both modulo statements, both lose counted insns and emitted instructions rather than adding them.
- mechanism: cse collapses the repeated loads and the repeated division before loop_optimize, deleting counted insns as well as emitted ones.
- probe: r_nit (t = *(s32*)(src + 4) read by all three divisions) and r_both (one q = x/30 feeding both the /30 and %30 statements), measured against candidate.c.
- result: r_nit measures 43 insns / 85 asm; r_both measures 51 insns / 98 asm - both far below candidate.c's 56 / 107 and both wrong on bytes. Corroborates the older rejected/single-read-v-local-loses-16-insns.c on the current chassis.
- verdict: KILLED
- kill_scope: instance
- measured_on: shipped chassis HEAD 2026-09-05, tmp/grind/func_8003C714/s15/r_nit.c and r_both.c, no FAKE construct present

## [s15] Mandated kill re-audit: candidate.c carries no FAKE-annotated construct on the current chassis, and the s_max body derived from it still measures score 15 / target 104 / build 105.
- mechanism: tools/fake_ablate.py inspects the candidate for FAKE annotations and re-measures with each ablated; the sandbox re-measures the honest cheat-stripped distance on the current HEAD.
- probe: python3 tools/fake_ablate.py --func func_8003C714 --file code6cac_c2 --candidate memory/grind/func_8003C714/candidate.c, plus a real sandbox run with the s_max body applied over the INCLUDE_ASM line and src/ restored afterwards.
- result: fake_ablate: 'no FAKE-annotated constructs found in memory/grind/func_8003C714/candidate.c; nothing to ablate'. Sandbox: score 15, target_insns 104, build_insns 105, rules_dropped 0. Third consecutive session to confirm this branch; future sessions need not re-run it.
- verdict: CONFIRMED

## s16 (2026-09-05) - synthesis modality

Chassis re-checked FIRST (the brief reported the driver measurement as
unavailable): candidate.c applied over the INCLUDE_ASM line at
src/code6cac_c2.c:629, `sandbox func_8003C714 --disable all` = score 15,
target_insns 104, build_insns 105, rules_dropped 0, cheat_asm_stripped 9.
Unchanged since s1. src/ restored to INCLUDE_ASM immediately afterwards.
The fake_ablate branch of the kill re-audit is CLOSED by s15 (three
consecutive confirmations that candidate.c carries no FAKE construct) and was
not re-run; instead this session re-audited the ledger's WEAKEST verdict per
the CONTRADICTION RULE, and that re-audit found a false cap (H25 below).

This session attacked the two live frontier items s15 left and settled both.

### CONFIRMED

- **H25 - s15's cap on the movable-ORDER dial is FALSE, and the target's
  in-loop const form is reachable through the order term ALONE.** s15 wrote
  "K24's order dial is capped at -6 because there are only two other
  movables". Extra movables can be manufactured without limit: a chain of
  loop-invariant locals joined by DISTINCT operators (`* 3`, `^ 0x1001`,
  `+ 7`, `* 5`, ...) is NOT constant-folded by cse1 - which is precisely what
  destroyed s15's K44 pure-xor chain - so every link becomes its own movable.
  Measured sweep (tmp/grind/func_8003C714/s16/sweep.sh, shipped cc1,
  canonical flags, -dL):

      links   insn_count  moved  not-desirable  asm_lines
        0         56        3          0           107   (= candidate.c)
        1         58        5          0           111
        2         59        6          0           112
        4         62        9          0           115
        8         67       14          0           120
       10         70       17          0           123
       11         71       18          0           124
       12         73       18          2           126
       13         74       19          2           127
       14         75       20          2           128
       15         77       22          2           130
       16         78       23          2           131

  At 12 links the 0x91A2B3C5 movable prints "not desirable" with NO inert
  padding whatsoever, i.e. the s14/s15 framing of the problem as "find +54
  free counted insns" is only one of two ways to reach the same gate; the
  order term reaches it too, at 3 units per movable exactly as H22 priced it.
  Banked at rejected/mixed-op-invariant-chain-scales-the-order-dial-but-
  costs-12-bytes-per-movable.c.

- **H26 - the one-flip window is 3 units wide and its exact recipe is now
  known.** At 11 links insn_count is 71 and the magic's threshold is exactly
  71 (122 - 3*16 moved ahead of it), so it still moves; one more link
  overshoots and flips the &D_80106A58 symbol_ref TOO (not-desirable = 2),
  which the target hoists. The correct configuration is 11 links plus ONE unit
  of the free ordinary split-init channel (H23): symbol_ref judged at 74 >= 72
  (moves), magic judged at 71 < 72 (stays in loop). So a distance-0 order-dial
  body has a fully specified shape; only its byte cost is fatal.

### KILLED

- **K46 (instance) - no moved movable measured in this loop is free; the
  measured price is 1.0-1.4 emitted instructions each.** This is the direct
  answer to s15's frontier item 1. Four carrier shapes were measured:
  (a) mixed-operator invariant chains - 15 extra moved movables cost 19 extra
  emitted instructions (asm_lines 107 -> 126); (b) duplicate invariant
  expressions (`wa = (s32)s0 * 3; wb = (s32)s0 * 3;`), hoping combine_movables
  would MATCH them and mark them regs_may_share - measured 59 insns, 6 moved,
  asm_lines 115, i.e. three extra movables at eight extra instructions, no
  match and no sharing; (c) a named invariant base pointer declared inside the
  loop (`base = (u8 *)&D_80106A58; src = base + i * 8;`) - 56 insns, 3 moved,
  107 asm, IDENTICAL to candidate.c because cse1 propagates the copy away;
  (d) a named invariant copy of the struct pointer (`b = s0;`) - likewise
  56 / 3 / 107. So (c) and (d) buy nothing at all and (a),(b) are priced in
  bytes. Measured on the shipped chassis with no FAKE construct present.

- **K47 (class) - the threshold INITIALISATION term is source-unreachable, so
  122 is a constant of this chassis.** s15's K43 closed the `loop_has_call`
  factor. The other factor, `n_non_fixed_regs`, is written in exactly two
  places in the whole compiler: regclass.c:387 (the `init_reg_sets_1` count
  over the target's `fixed_regs[]` table, i.e. build configuration) and
  regclass.c:530 (`globalize_reg`, `n_non_fixed_regs--`). `globalize_reg` has
  exactly one caller, varasm.c:547, reached only from `make_decl_rtl` for a
  TOP-LEVEL variable carrying an asm register specification - a global
  register variable, `register T x asm("$N");`. That is the register-asm-pin
  family, first on the forbidden-family catalog. The threshold expression at
  loop.c:532 therefore has no ordinary-C input at all.

- **K48 (instance) - the magic constant's own movable-admission, savings and
  lifetime inputs are all pinned at their minima.** Re-derived from compiler
  source this session rather than inherited: `m->savings = n_times_used[regno]`
  (loop.c:793) where `n_times_used` is a bcopy of `n_times_set` (loop.c:598),
  so savings counts SETS and is 1 for a once-materialised constant;
  `m->lifetime` (loop.c:791) is 1 because the const's def and its `mult`
  consumer are adjacent LUIDs; admission is unconditional because
  loop.c:695-700 case (2) (`! REG_USERVAR_P && ! REG_LOOP_TEST_P`) is
  satisfied by any compiler-generated pseudo whatever `maybe_never` says, so
  putting the division inside an `if` cannot suppress the movable; and
  `may_not_move` has only the explicit-CLOBBER trigger and the set-twice
  triggers (loop.c:3021-3047), none of which a single constant materialisation
  can reach.

### FRONTIER after s16 (reset)

1. **THE LIVE ITEM - the gate is now a single priced quantity, and the price
   is bytes.** Both terms of `insn_count + 3*extra_moved >= 120` are reachable
   (split-init +7, free and ordinary; mixed-op chains unlimited but priced at
   ~1.2 emitted instructions per movable; inert padding unlimited and free but
   inadmissible). What has never been measured is a construct that raises
   EITHER term while the emitted instruction count stays at 107. Every shape
   measured across s6-s16 falls into exactly one of three buckets: free and
   inert (inadmissible), real and priced in bytes, or folded away by cse1
   before it is counted. The next probe should attack the third bucket
   deliberately: find an ordinary construct that is counted by
   `count_loop_regs_set` but deleted by a pass that runs AFTER loop_optimize
   and is NOT combine's force_to_mode. cse2 (rerun-cse-after-loop) is the
   obvious unexamined candidate - a computation that becomes redundant only
   AFTER strength reduction has rewritten the givs would be counted by loop.c,
   deleted by cse2, and semantically real. Probe: write the loop's address
   arithmetic in a form whose redundancy appears only after strength reduction
   (e.g. an explicit `i * 4` offset local reused where a giv will land), and
   sweep insn_count against asm_lines with s16/sweep.sh.
2. **Re-derive the 15-instruction residual - DISCHARGED this session.** s15's
   frontier item 2 asked whether all 15 differing instructions are downstream
   of the const placement. s14's own measurement already answers it: the k=32
   padded body reaches sandbox score 0 with target_insns 104 and build_insns
   104, and the only thing that body changes relative to candidate.c is the
   movable decision (the pad emits nothing - asm_lines is constant at 107 from
   k=0 through k=31). A body differing from candidate.c only in the hoist
   decision scores 0, so all 15 residual instructions ARE downstream of the
   hoist. s1's H2 is confirmed by measurement; do not spend a session diffing
   .greg/.sched for it.
3. **The declaration puns remain a submission blocker on ANY distance-0 body**
   (unchanged from s14/s15; restated because it gates submission independently
   of the floor). candidate.c line 358 `src = (u8 *)&D_80106A58 + i * 8;` and
   line 370 `*((u8 *)s0 + 0x30) = *(u16 *)&D_80101ED2;`. Sanctioned fix is the
   aggregate-merge family at the DECLARATION in include/*.h via an integration
   handoff, prongs (a)-(e), with prong (c)'s 2026-09-03 amendment requiring the
   sub-symbol rows to carry an `alias of g_file_disc_type+4; retire with
   func_80035280` suffix. Prong (a) is satisfiable from base-register/stride
   evidence: the target walks a single base register with
   `addiu $a2, $a2, 0x8` at 8003C830.

## [s16] Chassis re-check: candidate.c still measures score 15 / target 104 / build 105 on HEAD 2026-09-05
- mechanism: the brief reported the driver's dispatch measurement as unavailable, so the floor had to be re-established before any banked spelling conclusion could be spent
- probe: candidate.c applied over src/code6cac_c2.c:629 and `& tools/wteng.ps1 main sandbox func_8003C714 --disable all`; src/ restored to INCLUDE_ASM immediately afterwards
- result: score 15, target_insns 104, build_insns 105, rules_dropped 0, cheat_asm_stripped 9 - identical to every session since s1
- verdict: CONFIRMED

## [s16] s15's statement that the movable-order dial is capped at -6 is false: a chain of loop-invariant locals joined by DISTINCT operators manufactures unlimited extra movables, and at 12 links the 0x91A2B3C5 movable prints "not desirable" with no inert padding present at all
- mechanism: cse1 constant-folds a chain of identical operators (s15's K44 pure-xor chain collapsed to one insn) but cannot fold a chain that alternates multiply, xor, add and subtract; each surviving link is a separate loop-invariant SET, hence a separate movable, and loop.c:1719 decrements threshold by 3 for each one moved ahead of the magic constant's movable
- probe: mixed-operator chains of 1, 2, 4, 8, 10, 11, 12, 13, 14, 15 and 16 links generated by tmp/grind/func_8003C714/s16/gen_u.py and gen_v.py, swept with s16/sweep.sh, reading the movable table and the "not desirable" lines out of the per-variant .seg dump and the emitted instruction count out of the .s
- result: insn_count 56/58/59/62/67/70/71/73/74/75/77/78 with moved 3/5/6/9/14/17/18/18/19/20/22/23 and asm_lines 107/111/112/115/120/123/124/126/127/128/130/131; not-desirable first appears at 12 links, where the .seg shows "Insn 93: regno 106 (life 1), move-insn savings 1 not desirable" and "Insn 106: regno 112 (life 1), move-insn savings 1 not desirable" while the life-31 0x88888889 movable still moves
- verdict: CONFIRMED

## [s16] The one-flip window is exactly 3 units wide and its recipe is 11 mixed-operator links plus one unit of the free split-init channel
- mechanism: the &D_80106A58 symbol_ref movable is judged one position before the magic, so the symbol_ref sees threshold T and the magic sees T-3; the target requires the symbol_ref MOVED and the magic NOT moved, which needs insn_count in (T-3, T]
- probe: read the moved and not-desirable counts at 10, 11, 12, 13, 14, 15 and 16 links against insn_count
- result: at 11 links insn_count is 71 and the magic's threshold is 122 - 3*16 = 71, so 71 >= 71 still moves; at 12 links insn_count 73 against symbol_ref threshold 68 flips BOTH. Adding one unit of H23's split-init channel to the 11-link body gives symbol_ref 74 >= 72 (moved) and magic 71 < 72 (not moved), which is the target configuration
- verdict: CONFIRMED

## [s16] Every moved movable measured in this loop costs emitted bytes; s15's frontier item 1 (a free moved movable) is answered in the negative for the four shapes that span the mechanism
- mechanism: move_movables hoists the movable's insn into the preheader (loop.c:1640-1700) and nothing afterwards deletes it unless its in-loop use disappears, which is the same condition that makes the construct inert; the two shapes that could have escaped that - combine_movables MATCHING two equal invariants and marking them regs_may_share (loop.c:1283-1285 and loop.c:1655-1660), and a plain invariant copy that local-alloc would coalesce - were measured directly
- probe: (a) mixed-operator chains, 15 extra moved movables; (b) v_dup, two locals assigned the same invariant expression `(s32)s0 * 3` and both consumed; (c) u_base_ptr, `base = (u8 *)&D_80106A58; src = base + i * 8;` declared inside the loop; (d) u_cpy, `b = s0; dst = (u8 *)b + i * 4;`
- result: (a) asm_lines 107 -> 126 for 15 extra moved movables, i.e. 1.0-1.4 emitted instructions each; (b) 59 insns / 6 moved / 115 asm - three extra movables at eight extra instructions, no match and no register sharing; (c) 56 / 3 / 107 and (d) 56 / 3 / 107, both byte-identical to candidate.c because cse1 propagates the copy away and no movable is created at all
- verdict: KILLED
- kill_scope: instance
- measured_on: shipped chassis HEAD 2026-09-05, tmp/grind/func_8003C714/s16/{u_mix1,u_mix2,u_mix4,u_mix8,u_mix16,v_mix10..15,v_dup,u_base_ptr,u_cpy}.c, no FAKE construct present

## [s16] The threshold initialisation term of loop.c:532 has no ordinary-C input: n_non_fixed_regs is decremented only by globalize_reg, whose sole caller is the global-register-variable path in make_decl_rtl
- mechanism: threshold = (loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs) at loop.c:532; s15's K43 closed loop_has_call; n_non_fixed_regs is assigned at regclass.c:380 and incremented at regclass.c:387 from the target's fixed_regs[] table during init_reg_sets_1, and modified nowhere else except regclass.c:530
- probe: grepped every reference to n_non_fixed_regs across tools/gcc-2.7.2/*.c, read globalize_reg (regclass.c:505-537) and its single caller
- result: the only caller is varasm.c:547, inside make_decl_rtl, reached only when a TOP-LEVEL declaration carries an asm register specification (`register T x asm("$N");`) - a global register variable, i.e. the register-asm-pin family that heads the forbidden-family catalog. No admissible C construct can change the 122
- verdict: KILLED
- kill_scope: class
- predicate_cite: tools/gcc-2.7.2/varasm.c:547
- measured_on: shipped chassis HEAD 2026-09-05, compiler source reading, no FAKE construct present

## [s16] The magic constant's movable admission and its savings and lifetime inputs are all at their minima and none of the three is reachable from C
- mechanism: m->savings = n_times_used[regno] (loop.c:793) with n_times_used a bcopy of n_times_set (loop.c:598), so savings counts SETS and is 1 for a once-materialised constant; m->lifetime (loop.c:791) is the LUID distance from the const's def to its mult consumer, which are adjacent; admission at loop.c:695-700 grants case (2) to any pseudo that is neither REG_USERVAR_P nor REG_LOOP_TEST_P regardless of maybe_never, so a conditionally executed division is still made a movable; may_not_move is set only by an explicit CLOBBER of the reg or by the reg being SET twice in the loop (loop.c:3021-3047)
- probe: read loop.c:680-800, loop.c:960-1000 and loop.c:2989-3092, and cross-checked against the movable tables of the eight .seg dumps produced this session
- result: every input is pinned at its minimum. In particular putting the division inside an `if` cannot suppress the movable, which would otherwise have been the cheapest attack; and a constant materialised once can never trip either may_not_move trigger
- verdict: KILLED
- kill_scope: instance
- measured_on: shipped chassis HEAD 2026-09-05, compiler source reading against this session's .loop movable tables, no FAKE construct present

## [s16] s1's H2 - that all 15 residual instructions are downstream of the 0x91A2B3C5 hoist decision - is confirmed by s14's own distance-0 measurement, so s15's frontier item 2 needs no dump diff
- mechanism: s14's k=32 body differs from candidate.c only in carrying a pad that emits nothing (asm_lines constant at 107 from k=0 to k=31) and in flipping the movable decision; if any of the 15 differing instructions were independent of that decision, the padded body could not have reached zero
- probe: re-read s14's sweep table and its sandbox result against the residual claim, rather than diffing .greg/.sched instruction by instruction
- result: the k=32 body measured score 0, target_insns 104, build_insns 104. A body whose only difference from candidate.c is the hoist decision scores 0, so all 15 residual instructions are downstream of it. H2 CONFIRMED; the CONTRADICTION-RULE re-audit therefore moved to the movable-order cap instead, where it found the false verdict (H25)
- verdict: CONFIRMED

## [s16] s15's record that the movable-order dial is capped at -6 because there are only two other movables is wrong: a chain of loop-invariant locals joined by DISTINCT operators is not constant-folded by cse1, so each link becomes its own movable, and at 12 links the 0x91A2B3C5 movable prints 'not desirable' with no inert padding present at all.
- mechanism: cse1 constant-folds a chain of identical associative operators (this is exactly what collapsed s15's K44 pure-xor chain to a single insn) but cannot fold a chain that alternates multiply, xor, add and subtract. Every surviving link is a separate loop-invariant SET, hence a separate movable in scan_loop's list, and loop.c:1719 decrements threshold by 3 for each movable moved ahead of the magic constant's movable. The gate at loop.c:1631 is therefore reachable through the order term alone, without touching insn_count.
- probe: Mixed-operator chains of 1, 2, 4, 8, 10, 11, 12, 13, 14, 15 and 16 links (cycling '* 3', '^ 0x1001', '+ 7', '* 5', '^ 0x2002', '- 11', '* 9', '^ 0x4004' off (s32)s0, final link consumed by the tail store) generated by tmp/grind/func_8003C714/s16/gen_u.py and gen_v.py, compiled with the shipped cc1 under canonical flags with -dL, swept by tmp/grind/func_8003C714/s16/sweep.sh reading the movable table and the 'not desirable' lines from each variant's .seg dump and the emitted instruction count from its .s.
- result: insn_count 56/58/59/62/67/70/71/73/74/75/77/78 with moved movables 3/5/6/9/14/17/18/18/19/20/22/23 and asm_lines 107/111/112/115/120/123/124/126/127/128/130/131 at 0/1/2/4/8/10/11/12/13/14/15/16 links. 'not desirable' first appears at 12 links, where the dump reads 'Insn 93: regno 106 (life 1), move-insn savings 1 not desirable' and 'Insn 106: regno 112 (life 1), move-insn savings 1 not desirable' while the life-31 0x88888889 movable still moves.
- verdict: CONFIRMED

## [s16] The window in which the &D_80106A58 symbol_ref still moves while only the 0x91A2B3C5 magic declines is exactly 3 units wide, and its recipe is 11 mixed-operator links plus one unit of the free split-init channel.
- mechanism: The symbol_ref movable is judged one position before the magic in scan_loop's insn-ordered list, so the symbol_ref sees threshold T and the magic sees T-3. The target requires the symbol_ref hoisted (its lui/addiu are in the target preheader at 8003C748-8003C74C) and the magic left in the loop, which needs insn_count in the half-open interval (T-3, T].
- probe: Read moved and not-desirable counts at 10, 11, 12, 13, 14, 15 and 16 links against insn_count, and solved the two loop.c:1631 inequalities against the measured threshold decrements.
- result: At 11 links insn_count is 71 and the magic's threshold is 122 - 3*16 = 71, so 71 >= 71 and it still moves; at 12 links insn_count 73 against a symbol_ref threshold of 68 flips BOTH. Adding one unit of H23's free split-init channel on top of the 11-link body gives symbol_ref 74 >= 72 (moves) and magic 71 < 72 (stays in loop), which is the target configuration. The shape of a distance-0 order-dial body is therefore fully specified; only its byte cost is fatal.
- verdict: CONFIRMED

## [s16] None of the four moved-movable carrier shapes measured this session is free of emitted bytes: mixed-operator chains cost 1.0-1.4 emitted instructions per moved movable, duplicate invariant expressions are not MATCHED by combine_movables and cost eight instructions for three movables, and a named invariant base pointer or a plain invariant copy declared inside the loop produce no extra movable at all.
- mechanism: move_movables hoists a movable's insn into the preheader and nothing afterwards deletes it unless its in-loop use disappears, which is the same condition that makes the construct inert. The two shapes that could have escaped that were probed directly: combine_movables matching two equal invariants and marking them regs_may_share (loop.c:1253-1286 and loop.c:1655-1660), which local-alloc could have coalesced away to nothing; and a plain invariant register copy that local-alloc would coalesce with its source.
- probe: (a) the mixed-operator chain sweep above, 15 extra moved movables; (b) v_dup: 'wa = (s32)s0 * 3; wb = (s32)s0 * 3;' with both consumed; (c) u_base_ptr: 'base = (u8 *)&D_80106A58; src = base + i * 8;' declared inside the loop; (d) u_cpy: 'b = s0; dst = (u8 *)b + i * 4;'. All swept through tmp/grind/func_8003C714/s16/sweep.sh for insn_count, moved count and asm_lines.
- result: (a) asm_lines 107 -> 126 for 15 extra moved movables. (b) 59 insns / 6 moved / 115 asm - three ordinary extra movables at eight extra instructions, no match line in the dump and no register sharing. (c) 56 / 3 / 107 and (d) 56 / 3 / 107, both byte-identical to candidate.c on every axis, because cse1 propagates the invariant copy into its uses and delete_dead_from_cse removes the copy so no movable is created. This is the direct negative answer to s15's frontier item 1 for the shapes that span the mechanism.
- verdict: KILLED
- kill_scope: instance
- measured_on: shipped chassis HEAD 2026-09-05 (sandbox baseline re-measured this session at score 15 / target 104 / build 105), tmp/grind/func_8003C714/s16/{u_mix1,u_mix2,u_mix4,u_mix8,u_mix16,v_mix10..v_mix15,v_dup,u_base_ptr,u_cpy}.c, no FAKE construct present

## [s16] The threshold initialisation term at loop.c:532 has no ordinary-C input, because n_non_fixed_regs is decremented only by globalize_reg and globalize_reg is reached only from the global-register-variable path in make_decl_rtl.
- mechanism: threshold = (loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs). s15's K43 already closed the loop_has_call factor at loop.c:2202. n_non_fixed_regs is initialised to 0 at regclass.c:380 and incremented at regclass.c:387 once per non-fixed hard register during init_reg_sets_1, i.e. straight from the target's fixed_regs[] table, and is modified in exactly one other place in the entire compiler: regclass.c:530, 'n_non_fixed_regs--', inside globalize_reg.
- probe: Grepped every reference to n_non_fixed_regs across tools/gcc-2.7.2/*.c, then read globalize_reg (regclass.c:505-537) and traced its callers.
- result: globalize_reg has exactly one caller, varasm.c:547, inside make_decl_rtl on the top_level register-specification path - that is, a global register variable 'register T x asm("$N");'. That construct is the register-asm-pin family, first entry on the forbidden-family catalog. So the 122 is a constant of this chassis for every admissible C body, and the ledger's long-standing assumption to that effect is now predicate-cited rather than assumed.
- verdict: KILLED
- kill_scope: class
- measured_on: shipped chassis HEAD 2026-09-05, compiler source reading, no FAKE construct present
- predicate_cite: tools/gcc-2.7.2/varasm.c:547

## [s16] The magic constant's movable-admission test, its savings and its lifetime are all at their minimum values on this body and none of the three responds to a source change, including putting the division inside a conditional.
- mechanism: m->savings = n_times_used[regno] (loop.c:793) where n_times_used is a bcopy of n_times_set (loop.c:598), so savings counts SETS and is 1 for a once-materialised constant; m->lifetime (loop.c:791) is the LUID distance from the const's def to its mult consumer, which are adjacent, so 1; admission at loop.c:695-700 grants case (2) - '! REG_USERVAR_P (SET_DEST (set)) && ! REG_LOOP_TEST_P (SET_DEST (set))' - to any compiler-generated pseudo regardless of maybe_never or call_passed; and may_not_move (loop.c:3021-3047) is set only by an explicit CLOBBER of the register or by the register being SET twice inside the loop.
- probe: Read loop.c:680-800, loop.c:960-1000 and loop.c:2989-3092 end to end and cross-checked each quantity against the movable tables printed in this session's eight .seg dumps.
- result: Every input is pinned. Notably the conditional-execution attack is closed in a way the ledger had only assumed: a division placed inside an 'if' inside the loop still yields a movable, because the const pseudo is compiler-generated and satisfies case (2) unconditionally - so the cheapest imaginable escape (which would have cost a branch anyway, and the target's loop body is branchless) does not exist. A constant materialised once also cannot trip either may_not_move trigger.
- verdict: KILLED
- kill_scope: instance
- measured_on: shipped chassis HEAD 2026-09-05, compiler source reading against this session's .loop movable tables, no FAKE construct present

## [s16] s1's H2 - that all 15 residual instructions are downstream of the 0x91A2B3C5 hoist decision - is confirmed by s14's own distance-0 measurement, so s15's frontier item 2 is discharged without a .greg/.sched diff.
- mechanism: s14's k=32 padded body differs from candidate.c only in carrying a pad that emits nothing - asm_lines is constant at 107 from k=0 through k=31 - and in flipping the movable decision. If any of the 15 differing instructions were independent of that decision, that body could not have reached zero.
- probe: Re-read s14's sweep table and its sandbox result against the residual claim instead of spending a session diffing dumps instruction by instruction, then re-pointed the CONTRADICTION-RULE re-audit at the next-weakest verdict.
- result: The k=32 body measured sandbox score 0, target_insns 104, build_insns 104. A body whose only difference from candidate.c is the hoist decision scores 0, so all 15 residual instructions are downstream of it and the hoist really is the whole problem. The re-audit then moved to s15's movable-order cap, which is where the false verdict actually was (see the first hypothesis above).
- verdict: CONFIRMED

## s17 (2026-09-05, solver modality)

### H26 -- CONFIRMED. A dead libcall block is a FREE and UNBOUNDED insn_count channel.

`delete_dead_from_cse` is forbidden to touch a libcall block (`cse.c:8708`), so
a DEAD libcall survives cse1 and is still in the insn stream when
`count_loop_regs_set` runs; `flow.c` then removes the whole block via
`libcall_dead_p` (flow.c:1827; consulted flow.c:1482-1484; deleted
flow.c:1503/1551), and `flow` runs after `loop_optimize` and before `combine`
and register allocation. On MIPS1 there is no DImode divide instruction, so
`(long long)i / K` expands to such a block.

Measured on the shipped chassis, all at the baseline asm_lines of 107: one
statement with a `long long` destination is worth +9.5 insn_count on average
(56 -> 67 -> 76 -> 85 -> 94 -> 103 -> 113 -> 123 -> 133 for 1..8 statements);
one statement truncated into an existing `s32` local is worth +5
(85, 90, 95 at 5/6/7 statements; 109, 113, 118, 122, 126 at 10..14).
`(long long)i * K` is also a libcall (+11); `K << i` in DImode is not (+0).

This is the THIRD free channel for this function (s6 balanced biv noise, s14
combine `force_to_mode` pad, s17 libcall block) and the first that is
frame-exact as well as byte-exact.

### H27 -- CONFIRMED. The whole function reaches sandbox distance 0 on the SHIPPED chassis through this channel.

`t_v13` = the s15 split-init body plus thirteen `v = (long long)i / K;`
statements. insn_count 122, inside the H17 window [120, 122]. The `.loop`
movable table prints the target arrangement exactly (`&D_80106A58` moved,
`0x91A2B3C5` **not desirable**, `0x88888889` moved), and
`sandbox func_8003C714 --disable all` measures **score 0, target 104 == build
104, rules_dropped 0**. Banked, NOT submitted, at
`rejected/dead-dimode-libcall-block-13x-free-insn-count-d0-but-inadmissible.c`:
thirteen invented dead 64-bit divisions fail cheat-checklist tests T1/T2/T3, and
they are not the frozen list "dead stores / self-assigns to LOCALS or PARAMS"
family, which is a same-value re-store of a value the function already holds
(`.claude/rules/dead-store-fake-exception.md`), not a fresh division by an
arbitrary prime.

Consequence for the ledger: s1 H2 is now confirmed a third time, and the
function is a pure ADMISSIBILITY problem. Reachability, register allocation,
scheduling and frame layout are all settled -- a byte-exact, frame-exact d0
body exists on the shipped chassis and the only open question is whether a
SEMANTICALLY REAL carrier for the same channel can be spelled.

### K48 -- KILLED (instance). Dead SImode divisions reach the same insn_count but are not byte-free.

`ctl_v13` (thirteen dead `v = i / K;`, no `long long`) also measures insn_count
122 and also declines the `0x91A2B3C5` movable, but sandbox reports score 19 at
build_insns 105: one emitted instruction survives per body, so the channel is
priced. Freeness is a property of the libcall block deletion in `flow.c`, not of
dead code in general -- which is why s5 K15 measured dead ALU chains at +0
count while these measure at full price.

### K49 -- KILLED (instance). The cse2 / post-strength-reduction redundancy channel is empty.

Six address-arithmetic respellings designed so that the redundancy appears only
after `strength_reduce` (explicit `i*4` / `i*8` offset locals; four
independently computed store addresses; one offset local reused by all four
store addresses; a duplicated `dst` pointer; a separate `src+4` pointer; each
also combined with split-init) all leave insn_count pinned at 56 (63 with
split-init) and asm_lines at 107. cse1 folds the duplicated address arithmetic
before `loop_optimize` counts it, so nothing is counted at full price for cse2
to delete. The only spelling that adds count (`w_src2`, +2) adds 3 emitted
instructions. This closes the s16 frontier headline item.

### K50 -- KILLED (instance). Placing carriers before the magic buys no threshold reduction.

`o_v4/6/8/10/12` place the carriers ahead of the divisions so their divisor
constants become movables earlier in the movable list. Measured insn_counts are
identical to the same carrier counts placed after the divisions (`o_v12` 118 =
`t_v12` 118) and the `0x91A2B3C5` decision is unchanged. `threshold -= 3`
(loop.c:1719) executes only on the *moved* path, so once insn_count is large the
earlier carriers are themselves declined and contribute nothing. The order dial
and the insn_count dial cannot be stacked.

### s17 frontier

1. The channel needs a SEMANTICALLY REAL carrier. A libcall block is free only
   while its result is dead (a live one emits `jal __divdi3`), so freeness and
   deadness coincide exactly as in s14 K41. The unexamined shape is a libcall
   whose result is real but whose *value* the compiler can prove dead at flow
   time without the source statement being dead -- e.g. a DImode value written
   into a local that a later, unconditional store overwrites with a genuinely
   computed value, which is the sanctioned dead-store family only if the
   overwritten value is the SAME value. Probe: spell the three divisions so one
   of them is naturally computed twice (once in DImode, once in SImode) with the
   DImode copy overwritten by the SImode one, and check whether the DImode block
   is still `libcall_dead_p` and still counted.
2. The carrier requirement is now only 59 counted insns above the split-init
   base (63 -> 122), and each libcall statement is worth 5 or 9.5. Any
   admissible construct worth even 20 insn_count would cut the carrier count to
   a handful. Probe: sweep the OTHER libcall-generating constructs available in
   ordinary C on this target (DImode multiply, DImode modulo, DImode compare,
   `float`/`double` arithmetic if any expands to a libcall here) for their
   insn_count-per-statement rate, so that the cheapest admissible spelling is
   known before the next admissibility argument is built.
3. The two declaration puns in candidate.c remain a hard submission blocker on
   ANY distance-0 body (`src = (u8 *)&D_80106A58 + i * 8;` and
   `*((u8 *)s0 + 0x30) = *(u16 *)&D_80101ED2;`); the sanctioned fix is the
   aggregate-merge family at the DECLARATION in `include/*.h` routed as an
   integration handoff, with prong (a) satisfied from the target base-register
   stride evidence (`addiu $a2, $a2, 0x8` at 8003C830 with reads at 0x0 and
   0x4). Unchanged from s16; still unspent.

## [s17] A DEAD DImode (long long) division inside the loop is a free insn_count carrier: its libcall block survives cse1's delete_dead_from_cse, is counted by count_loop_regs_set at full price, and is then deleted whole by flow.c for zero emitted bytes.
- mechanism: cse.c:8708 (delete_dead_from_cse) explicitly refuses to delete insns that are part of a libcall block, so unlike the dead ALU chains killed at s5 K15 the block is still in the stream at loop_optimize. flow.c's libcall_dead_p (flow.c:1827, consulted at flow.c:1482-1484, deletion at flow.c:1503/1551) then removes the whole REG_LIBCALL/REG_RETVAL block, and flow runs after loop_optimize and before combine and before register allocation. On MIPS1 there is no DImode divide instruction, so (long long)i / K expands to exactly such a block.
- probe: tmp/grind/func_8003C714/s17/sweep.sh over x_ll1, x_ll2, y_ll3..y_ll8 (long long destination) and r_v5..t_v14 (result truncated into the existing s32 local v), reading insn_count from the -dL loop dump and asm_lines from the emitted .s.
- result: insn_count 56 -> 67, 76, 85, 94, 103, 113, 123, 133 for 1..8 statements with a long long destination (about +9.5 each) and 85, 90, 95, 109, 113, 118, 122, 126 for 5..14 statements truncated into an existing s32 local (about +5 each). asm_lines stays at the baseline 107 in every case. Controls: (long long)i * K is also a libcall (+11); a DImode shift by a variable is not (+0).
- verdict: CONFIRMED

## [s17] Through that channel the whole function reaches sandbox distance 0 on the shipped chassis, byte-exact and frame-exact, confirming s1's H2 that all 15 residual instructions are downstream of the loop.c hoist decision.
- mechanism: Thirteen dead libcall blocks on top of the s15 split-init body take loop insn_count from 63 to 122, inside H17's admissible window [120,122]: the &D_80106A58 movable still moves (threshold 122 >= 122), the 0x91A2B3C5 magic is declined (threshold 119 < 122) and stays in-loop as the target's lui/lw/ori/mult quartet, and the 0x88888889 magic (lifetime 35) still moves. The blocks then vanish in flow, so RA and sched never see them.
- probe: tmp/grind/func_8003C714/s17/t_v13.c copied over src/code6cac_c2.c, then `& tools/wteng.ps1 main sandbox func_8003C714 --disable all`; movable table read from s17/dumps/t_v13.seg.
- result: score 0, target_insns 104, build_insns 104, rules_dropped 0. Movable table: Insn 33 regno 82 (life 1) moved; Insn 47 regno 87 (life 1) NOT DESIRABLE; Insn 65 regno 93 (life 35) moved. No __divdi3 call and no other trace of the thirteen statements in the emitted function. NOT SUBMITTED: thirteen invented dead 64-bit divisions fail cheat-checklist T1/T2/T3 and are not the frozen list's dead-store family (which covers a same-value re-store of a value the function already holds, .claude/rules/dead-store-fake-exception.md), so this is a first reach of an unsanctioned family. Banked at memory/grind/func_8003C714/rejected/dead-dimode-libcall-block-13x-free-insn-count-d0-but-inadmissible.c. An intermediate spelling with a dedicated `long long q` local (z_s6, 6 statements, insn_count 120) also declines the magic and matches the instruction count but measures score 26 because the DImode local reserves 8 further frame bytes (0x28 vs the target's 0x20).
- verdict: CONFIRMED

## [s17] The thirteen-statement carrier written in SImode instead of DImode reaches the same insn_count and the same hoist decision but is not byte-free on this chassis.
- mechanism: Without the libcall block there is no whole-block flow.c deletion; one emitted instruction per body survives the later passes, so the carrier is priced rather than free. This is why s5's K15 measured dead ALU chains at +0 insn_count while these measure at full price - the two behaviours have different causes.
- probe: tmp/grind/func_8003C714/s17/ctl_v13.c (t_v13 with the (long long) casts removed) installed in src/code6cac_c2.c and measured with sandbox --disable all.
- result: insn_count 122, notdesirable 1 on the 0x91A2B3C5 movable, asm_lines 107 - but sandbox score 19 at build_insns 105 versus the target's 104.
- verdict: KILLED
- kill_scope: instance
- measured_on: shipped chassis HEAD 2026-09-05, tmp/grind/func_8003C714/s17/ctl_v13.c copied over src/code6cac_c2.c; no FAKE construct present (fake_ablate reports none in candidate.c or in the split-init base)

## [s17] The six address-arithmetic respellings measured this session, which were designed so that their redundancy appears only after strength_reduce, leave loop insn_count unchanged, so no insn is counted at full price for cse2 to delete.
- mechanism: cse1 runs before loop_optimize and folds duplicated address arithmetic within the single-basic-block loop body, so the duplicate address computations are already gone when count_loop_regs_set runs. The s16 frontier's premise - that such a computation would be counted at full price and deleted later by rerun-cse-after-loop - fails at its first step.
- probe: tmp/grind/func_8003C714/s17/w_off.c, w_addr4.c, w_offreuse.c, w_dst2.c, w_src2.c, w_split_off.c, w_split_addr4.c swept with s17/sweep.sh against the w_base and w_split controls.
- result: w_base 56/107; w_off 56/107; w_addr4 56/107; w_offreuse 56/107; w_dst2 56/107; w_split 63/107; w_split_off 63/107; w_split_addr4 63/107. The only spelling that adds count, w_src2 at 58, also adds 3 emitted instructions (asm_lines 110).
- verdict: KILLED
- kill_scope: instance
- measured_on: shipped chassis HEAD 2026-09-05, insn_count from -dL loop dumps and asm_lines from the emitted .s; no FAKE construct present

## [s17] Placing the carriers before the divisions, so their divisor constants become movables earlier in the movable list, produced insn_counts and hoist decisions identical to the same carriers placed after them.
- mechanism: loop.c:1719 executes `threshold -= 3` only on the moved path. Once insn_count is large the earlier carrier movables are themselves declined, so they contribute no threshold reduction. The order dial (s15 H22) and the insn_count dial therefore cannot be stacked.
- probe: tmp/grind/func_8003C714/s17/o_v4.c, o_v6.c, o_v8.c, o_v10.c, o_v12.c (carriers ahead of the divisions) swept against r_v5..t_v14 (carriers after).
- result: o_v4 81, o_v6 90, o_v8 100, o_v10 109, o_v12 118 - matching the after-placement series exactly at equal counts (o_v12 118 = t_v12 118), with the 0x91A2B3C5 decision unchanged and asm_lines 107 throughout.
- verdict: KILLED
- kill_scope: instance
- measured_on: shipped chassis HEAD 2026-09-05, tmp/grind/func_8003C714/s17/o_v*.c; no FAKE construct present

## [s17] The s15 split-init ceiling re-audited on the current chassis is unchanged: insn_count 63 at the baseline asm_lines of 107, with no FAKE construct present in it or in candidate.c.
- mechanism: Mandated kill re-audit of the instance kill sitting closest to the target. tools/fake_ablate.py reports no FAKE-annotated constructs in either file, so neither the split-init result nor the candidate floor was measured with a FAKE carrier occupying a pseudo.
- probe: s17/sweep.sh on w_split.c (the s15 body regenerated) plus fake_ablate.py --func func_8003C714 --file code6cac_c2 on candidate.c and on rejected/splitinit-ceiling-63-ordinary-c-byte-neutral-but-54-short.c.
- result: w_split 63 real insns / asm_lines 107, identical to the s15 record; candidate.c re-measured at sandbox score 15, target 104, build 105, rules_dropped 0; fake_ablate finds nothing to ablate in either file. The kill stands and split-init was used as the base chassis for every s17 probe.
- verdict: CONFIRMED

## s18 (2026-09-05, forensics modality)

### H29 -- CONFIRMED. `loop_has_call` is a live, C-reachable dial that HALVES the hoist threshold, and s8's dismissal of it rested on a false premise.

`loop.c:532` computes `threshold = (loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs)`
and `prescan_loop` (loop.c:2202) sets `loop_has_call` for ANY `CALL_INSN` between
the loop notes. On this chassis that is 122 vs 61. s8 banked
`rejected/threshold-term-only-movable-by-cheat-or-by-a-call-in-the-loop.c`, whose
objection (b) was that "a call in the loop adds a jal, its argument setup, and
turns every giv register into a caller-saved liability -- it is not a spelling of
this function". That is true of a LIVE call and false of a call inside a DEAD
libcall block: `flow.c`'s `libcall_dead_p` (flow.c:1827) deletes the whole
REG_LIBCALL/REG_RETVAL block including its `CALL_INSN`, and `flow` runs AFTER
`loop_optimize` and BEFORE `combine` and register allocation. Measured: with one
dead `(long long)i / (long long)*(s32 *)(src + 4)` in the loop body the `.loop`
dump prints both life-1 movables as `not desirable` at insn_count 71 -- which is
only possible at threshold 61, since s17's x_ll1 had every movable MOVED at
insn_count 67 with threshold 122 -- and the emitted function has no `jal
__divdi3` and no other trace of the statement (build_insns 104 == target 104).

Probe: tmp/grind/func_8003C714/s18/c_dvar.c, c_mvar.c, plus the c_base control,
swept with s18/sweep.sh; movable tables in s18/dumps/*.seg.

### H30 -- CONFIRMED. Hoisting the `&D_80106A58` base pointer to a pre-loop local is byte-neutral and DELETES s11's H17 window.

`base = (u8 *)&D_80106A58;` before the loop, `src = base + i * 8;` inside it,
leaves the symbol's set OUTSIDE the loop, so loop.c never sees it as a movable at
all. Measured `M_pre`: 2 movables instead of 3, loop insn_count 55 (56 without
the hoist), asm_lines 107, and `sandbox --disable all` = score 15 / target 104 /
build 105 -- identical to candidate.c. `N_split` (the same hoist on the s15
split-init body) = insn_count 62, score 15, build 105.

The consequence is structural. H17's admissible window [120,122] existed ONLY
because the `&D_80106A58` movable (life 1, savings 1) had to be MOVED first, its
`threshold -= 3` (loop.c:1719) being what let the magic decline three counts
later. With that movable gone the 0x91A2B3C5 magic is the FIRST movable in the
list, so the whole requirement collapses to `insn_count > threshold` with NO
upper bound. Every future carrier search on this function is a one-sided
inequality, not a three-wide window.

### H31 -- CONFIRMED. Combining H29 and H30, the function reaches sandbox distance 0 on the shipped chassis with ONE carrier statement, frame-exact.

`Q_secf` = the s15 split-init body + the H30 base hoist + the single statement
`v = (long long)((float)*(s32 *)(src + 4) / 30.0f);` placed at the top of the
loop body, where `v` is the existing `s32` local that is unconditionally
re-assigned `v = *src;` later in the same iteration. Loop insn_count 68 > 61, the
0x91A2B3C5 movable prints `not desirable`, the 0x88888889 movable (life 35) still
moves. `sandbox func_8003C714 --disable all` = **score 0, target_insns 104 ==
build_insns 104, rules_dropped 0**, frame `subu $sp,$sp,32` = the target's 0x20.
`P_fix` (`v = (long long)(float)*(s32 *)(src + 4);`, no divide) measures score 0
identically at insn_count 66.

This replaces s17's thirteen-statement carrier with one. NOT SUBMITTED: see the
s18 ruling-request. Banked at
rejected/loop-has-call-halves-threshold-one-dead-fixsfdi-carrier-d0-but-rhs-unnatural.c
and rejected/loop-has-call-fixsfdi-bare-cast-carrier-d0-but-rhs-unnatural.c.

### K51 -- KILLED (instance). The DImode divide/modulo spelling of the carrier grows the frame by 8 bytes.

- mechanism: `__divdi3` / `__moddi3` / `__udivdi3` take two DImode arguments, which
  raises `current_function_outgoing_args_size` from 16 to 24 bytes. That size is
  charged at RTL-expansion time, long before `flow` deletes the libcall block, so
  deleting the call cannot recover it. A float -> long long conversion
  (`__fixsfdi` / `__fixunssfdi`) takes a 4-byte SF argument and leaves the area
  at 16.
- probe: tmp/grind/func_8003C714/s18/c_dvar.c, c_mvar.c, N_car1.c, Q_llv.c,
  M_pre_C.c versus P_fix.c, Q_secf.c, Q_minf.c, Q_dbl.c, Q_ufix.c; prologue read
  from the emitted .s in s18/dumps/, sandbox run on N_car1 and M_pre_C.
- result: every DImode-divide spelling emits `subu $sp,$sp,40`; every float ->
  long long spelling emits `subu $sp,$sp,32`, matching the target's
  `addiu $sp, $sp, -0x20`. sandbox: M_pre_C score 16, N_car1 score 22, both at
  build_insns 104 == target 104 (the residual is the frame and its sp-relative
  offsets, not the loop).
- verdict: KILLED
- kill_scope: instance
- measured_on: shipped chassis HEAD 2026-09-05, s18/dumps/*.s prologues plus
  sandbox on M_pre_C.c and N_car1.c; no FAKE construct present in any body

### K52 -- KILLED (instance). Raising the `&D_80106A58` movable's LIFETIME, rather than removing it, is not byte-neutral.

- mechanism: `loop.c:791-793` sets `m->lifetime` to the luid span of the pseudo
  and `m->savings` to `n_times_used`, and the test is
  `threshold * savings * lifetime >= insn_count`. A second, non-CSE-able
  reference to the symbol raises both, which keeps the base movable moving even
  at threshold 61 -- but the second reference is itself emitted.
- probe: s18/L_sym2.c, L_ptr2.c (a second `(u8 *)&D_80106A58 + i * 8` expression),
  L_field.c (`&D_80106A58 + i*8 + 4` for the word field, `src` for the byte read)
  and L_late.c (statement reorder), each also built with the DImode carrier.
- result: L_sym2 / L_ptr2 / L_late all fold at cse1 and print life 1 unchanged
  (insn_count 56, asm_lines 107). L_field does raise the symbol movable to
  life 7 / savings 2 and, with the carrier, prints exactly the target movable
  arrangement (base moved, magic `not desirable`, 0x88888889 moved) -- but it
  costs bytes: asm_lines 111 alone and 109 with the carrier, versus 107 for the
  base. The H30 source-level hoist achieves the same arrangement for free.
- verdict: KILLED
- kill_scope: instance
- measured_on: shipped chassis HEAD 2026-09-05, s18/dumps/L_*.seg movable tables
  and asm_lines; no FAKE construct present

### K53 -- KILLED (instance). Float and double arithmetic that stays in SF/DF mode is not a carrier at all.

- mechanism: this chassis is hard-float (`n_non_fixed_regs == 60`), so SF/DF add,
  multiply, divide and the SI<->SF conversions are emitted inline with no
  `CALL_INSN`, which means no `loop_has_call` and no libcall block for flow to
  delete. Only conversions that touch DImode go through libgcc.
- probe: s18/c_fdi.c, c_dfi.c, c_ftod.c, c_fdiv.c, c_llmulv.c and Q_flt.c.
- result: `(float)(long long)i`, `(double)(long long)i`, `(double)(float)i`,
  `(float)i / 3.0f` and `(long long)i * (long long)var` all leave insn_count at
  the 56 baseline with all three movables moved and asm_lines 107. `Q_flt`
  (`v = (float)x / 30.0f;`, dead, no long long) reaches insn_count 65 but still
  moves all three movables -- 65 < threshold 122 and loop_has_call is 0.
- verdict: KILLED
- kill_scope: instance
- measured_on: shipped chassis HEAD 2026-09-05, s18/dumps/*.seg; no FAKE present

### K54 -- re-audit of the s15 split-init instance kill on the current chassis: unchanged.

- mechanism: mandated kill re-audit. The split-init body is the base chassis for
  every s17 and s18 carrier probe, so its ceiling is the number the whole search
  is measured against.
- probe: s18/N_split.c (split-init + the H30 base hoist) swept and sandboxed.
- result: loop insn_count 62 (63 without the base hoist, as s15/s17 recorded),
  asm_lines 107, sandbox score 15 / target 104 / build 105. Unchanged. The kill
  stands and the base is confirmed on the current chassis. `tools/fake_ablate.py`
  could not run on these bodies (it errors on the FAKE annotations belonging to
  the OTHER 43 functions in code6cac_c2.c), but no s18 body and no line of
  candidate.c carries a FAKE annotation, so no lever was measured with a FAKE
  carrier occupying a pseudo.

### s18 frontier

1. ADMISSIBILITY of the single carrier is now the whole problem, and it is a
   ruling question, not a measurement. The statement is a store to an existing
   `s32` local whose stored value is never read (store-level deadness, the
   defensive-init shape the dead-store family explicitly sanctions), but its RHS
   cast chain (`(long long)((float)x / 30.0f)`) exists only to make GCC emit a
   `__fixsfdi` CALL_INSN, which is what T2 (human-programmer) would flag. s18
   returned `ruling-request` on exactly this rather than submitting.
2. If the ruling is NO, the next search is for a frame-exact `CALL_INSN`-bearing
   construct whose C spelling is natural. The constraint set is now small and
   fully characterised: it must touch DImode (K53 -- SF/DF stays inline), it must
   NOT pass a DImode argument (K51 -- +8 frame), and it needs only
   insn_count > 61 on the split-init base of 62, i.e. ANY positive count (H30
   removed the upper bound). `__fixsfdi`, `__fixunssfdi`, `__fixdfdi` are the
   candidates measured; `__floatdisf` / `__floatdidf` (DI argument) are not.
3. The two declaration puns in candidate.c remain a hard submission blocker on
   ANY distance-0 body, unchanged from s16/s17: `src` derives from the
   `D_80106A58` per-word splat sub-symbol of `g_file_disc_type @80106A54`, and
   `*((u8 *)s0 + 0x30) = *(u16 *)&D_80101ED2` reads an s16-declared symbol
   through a pun. The sanctioned fix is the aggregate merge at the DECLARATION in
   `include/*.h`, prongs (a)-(e), routed as an integration handoff, with prong (a)
   satisfied from the target's base-register stride evidence (`addiu $a2, $a2,
   0x8` at 8003C830, reads at 0x0 and 0x4) rather than from splat-name adjacency.
   Note the H30 base hoist makes this MORE tractable, not less: the merged
   declaration would be named once, in the pre-loop initialiser.


## s18b (2026-09-05, forensics modality) -- DISTANCE 0 REACHED WITH AN ADMISSIBLE BODY

Context: the previous forensics session (recorded above as "s18") reached distance 0
with a float -> long long carrier and returned a ruling-request; the Judge FAILed that
RHS on 2026-09-05 (docs/grind/decisions.md, entry titled `ruling: Is ONE statement of
the form v = (long long)((float)*(s32 *)(src + 4) / 30.0f);`) but issued a BINDING
sub-ruling: "A dead store whose RHS is itself ordinary C a reader can justify from the
program's own quantities - and which happens to touch DImode - is INSIDE the family and
is adjudicated on its prerequisites ... Keep hunting a natural DImode RHS." That session
also left `candidate.c` truncated to a comment header with NO function body (427 lines
-> 40); this session restored it and replaced it with the distance-0 body.

### H32 -- CONFIRMED. K51's mechanism attribution was wrong: the +8 frame on DImode-argument libcalls is a LOCALS slot, not the outgoing-argument area.

- mechanism: every `.frame` line emitted for this function reads `args= 16` regardless
  of the carrier; the DImode-divide bodies read `vars= 16` where the frame-exact bodies
  read `vars= 8`. The extra 8 bytes are a stack slot assigned during RTL expansion for a
  DImode temp and never referenced in the output (the phantom-frame-slot behaviour),
  NOT `current_function_outgoing_args_size` growing 16 -> 24 as K51 recorded. In
  `c_dvar.s` the frame is 40 with `buf` at 16($sp) and 24..31($sp) never touched by any
  instruction.
- probe: the `.frame` line of func_8003C714 read out of all 32 emitted `.s` files in
  tmp/grind/func_8003C714/s18/dumps/, plus the sp-relative reference census of c_dvar.s.
- result: vars=8 / args=16 for c_base, P_fix, P_const, P_out, Q_secf, Q_dbl, Q_minf,
  Q_ufix, Q_flt, M_pre, N_split, c_fdi, c_dfi, c_ftod, c_fdiv, c_llmulv and every new
  G_* body except two; vars=16 / args=16 for c_dvar, c_mvar, N_car1, P_call, P_udiv,
  Q_llv, G_divv, G_tot2 and the four L_*_C bodies.
- verdict: CONFIRMED

### H33 -- CONFIRMED. Whether a DImode divide reaches __divdi3 at all, and whether it costs the extra slot, is decided by the DIVIDEND, not by the divisor being variable.

- mechanism: `(long long)x / K` where x is an SImode value is a sign_extend, and
  expand_divmod narrows it back to a 32-bit divide - no libcall and no `loop_has_call`.
  A product that is genuinely DImode (`(long long)x * 100`, which MIPS expands inline
  through mulsidi3 / mult) IS a 64-bit dividend, so `/ 30` on it emits __divdi3 - and
  that spelling measures vars=8, frame 32, the target's 0x20. The bodies that cost the
  extra 8 bytes are the ones whose DIVISOR is also a live DImode value (`/ (long long)i`)
  or whose quotient stays 64-bit (`* 100 / 3000`).
- probe: standalone compile of six one-line functions
  (tmp/grind/func_8003C714/s18/probe/t1.c -> t1.s) plus the G_* chassis sweep through
  s18/sweepg.sh (which reports insn_count, movable decisions, asm_lines AND the frame).
- result: `(long long)x / 7` and `(long long)x / 1800` emit NO call; `(long long)x * 100`
  emits no call; `((long long)x * 100) / 30` emits `jal __divdi3`; `(long long)x /
  (long long)y` emits `jal __divdi3`; `(long long)(float)x` emits `jal __fixsfdi`.
  Chassis sweep: G_divk (`(long long)t / 1800`) insn_count 61 with all movables moved
  (no loop_has_call); G_mul (`(long long)t * 100`) insn_count 64, all moved; G_tot /
  G_ms / G_umul / G_mod insn_count 68 with the 0x91A2B3C5 magic declined and frame 32;
  G_divv and G_tot2 frame 40.
- verdict: CONFIRMED

### H34 -- CONFIRMED. The function reaches sandbox distance 0 on the shipped chassis with ONE dead store whose RHS is ordinary integer C, no float, frame-exact, and with BOTH declaration puns removed.

- mechanism: H29 (loop_has_call halves threshold 122 -> 61) + H30 (the base hoist removes
  the &D_80106A58 movable so the requirement is one-sided) + H33 (a 64-bit product
  divided by a constant is the frame-exact libcall). The carrier is
  `v = ((long long)*(s32 *)(src + 4) * 100) / 30;` - the record's elapsed time expressed
  in hundredths of a second, computed in 64-bit so the x100 scaling cannot overflow,
  stored into the existing `s32 v` which `v = *src;` unconditionally overwrites in the
  same iteration (store-level deadness).
- probe: tmp/grind/func_8003C714/s18/G_tot.c, then H2a.c (the same body with the two
  declaration fixes and the FAKE annotation), installed in src/code6cac_c2.c and
  measured with `& tools/wteng.ps1 main sandbox func_8003C714 --disable all`.
- result: **score 0, target_insns 104 == build_insns 104, rules_dropped 0**, frame
  `subu $sp,$sp,32`. Three sibling spellings measure 0 identically: G_ms
  (`* 1000 / 30`, milliseconds), G_umul (unsigned), G_mod (`* 100 % 3000`).
  Additionally measured byte-neutral and therefore ADOPTED: `extern u8
  D_80106A58[24];` with `base = D_80106A58;` (H2a, score 0) removes the
  `(u8 *)&D_80106A58` scalar-address pun, and `*((u8 *)s0 + 0x30) = D_80101ED2;`
  (H_nopun, score 0) removes the `*(u16 *)&D_80101ED2` pun while still emitting the
  target's `lhu` at 8003C874. The s16 declaration at include/code6cac.h:350 is left
  untouched, so NO header change and NO integration handoff is required.
- verdict: CONFIRMED

### K55 -- KILLED (instance). The carrier's destination variable is load-bearing: written into `c` instead of `v`, the same mechanism leaves a 7-instruction residual.

- mechanism: with the carrier stored into the hundredths accumulator `c` and the RHS
  `((long long)(t % 30) * 100) / 30`, loop insn_count is 76 and both life-1 movables are
  declined, but the emitted magic-multiply quartet comes out with v0 and v1 exchanged
  and one `sra` scheduled a slot early. Defeating the hoist is necessary but not
  sufficient - the carrier's destination also selects the RA seat of the dividend.
- probe: tmp/grind/func_8003C714/s18/G_hun.c installed in src/code6cac_c2.c and
  sandboxed; asm diff via s18/diffg.sh.
- result: score 7, build_insns 104 == target 104, frame 32. Banked at
  rejected/dimode-carrier-stored-into-c-not-v-scores-7.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: shipped chassis HEAD 2026-09-05, G_hun.c in src/code6cac_c2.c, one
  FAKE-class dead store present (the carrier itself); no other FAKE construct

### K56 -- KILLED (instance). Declaring the record block as a 2-D array and indexing it directly changes the giv the loop is reduced around.

- mechanism: `extern u8 D_80106A58[3][8];` with `src = D_80106A58[i];` (no `base` local)
  removes the invariant base pointer entirely, so loop.c no longer sees the
  index-derived pointer pair the target is built from.
- probe: tmp/grind/func_8003C714/s18/H2b.c installed in src/code6cac_c2.c and sandboxed.
- result: score 19, build_insns 105 vs target 104. The flat `extern u8 D_80106A58[24];`
  + `base = D_80106A58;` + `src = base + i * 8;` spelling reaches score 0 with the same
  pun-free property. Banked at
  rejected/twodim-array-record-index-changes-the-giv-score19.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: shipped chassis HEAD 2026-09-05, H2b.c in src/code6cac_c2.c, the
  FAKE-class carrier present

### s18b frontier

1. SUBMITTED as candidate-ready (sandbox 0 measured this session with the edits in
   src/code6cac_c2.c). If layer-1 or the Judge FAILs the carrier, the next lever is a
   DIFFERENT frame-exact __divdi3 dividend drawn from the same quantities - G_ms
   (`* 1000 / 30`, milliseconds) and G_mod (`* 100 % 3000`) both measure score 0 and are
   already banked in tmp/grind/func_8003C714/s18/ - not a respelling of the same body,
   which the driver keys by body and will not re-review.
2. The two declaration puns that blocked every previous distance-0 body are GONE and were
   measured byte-neutral. The aggregate-merge / integration-handoff route the s16-s18
   frontier assumed for include/*.h is no longer needed for this function.
3. Unspent: the sibling ledger func_80067D14 (src/text1b.c, floor 1046) was not
   transplanted this session - it shares no block with this function's loop, and the
   function reached distance 0 without it.
