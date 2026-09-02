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
