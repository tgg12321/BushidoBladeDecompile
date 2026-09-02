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
