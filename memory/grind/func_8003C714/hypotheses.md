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
