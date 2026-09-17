# SELF-VET — func_80073200

Measured state of the diff being vetted: `sandbox func_80073200 --disable all`
= **score 0**, `target_insns 203 == build_insns 203`; `--diff` reports
**0 source-level, 0 operand-only**, and 6 not-scored hunks (all masked
branch-target relocation addends).

CONSTRUCTS: (1) local struct `S73200 s` with named fields sp18..sp3C (s32) and sp40..sp43 (u8), address-taken and passed to func_80073728/func_8007352C; (2) ordinary named locals `ctx`, `base1`, `base2`, `tmp`, `v1`, `idx`, each written once and consumed by the next statement; (3) one local `s1` reused (written 3×, read immediately after each write) for the three successive `<table base> + 0xC` display-list pointers; (4) `s.sp43 = 0x14;` written in BOTH arms of the colour if/else instead of once at the join — FAKE-annotated; (5) `cond = D_800A3580;` a fresh once-written / once-read named intermediate declared ahead of the `*(s32 *)(arg0 + 0x18) += 0xC;` bump and consumed by `if (cond < 2)` — FAKE-annotated.

## T1 semantic purpose:
- (1) struct: yes — it IS the PsyQ primitive the function fills in and passes by
  address; every field is stored and read by the callees. The u8 width on
  sp40..sp43 is a data-model fact, not a codegen device: `s8` makes GCC emit
  `li v0,-68 / li v0,-88` where the target emits `li v0,188 / li v0,168`
  (asm/funcs/func_80073200.s:50,55), i.e. the original fields are unsigned char.
- (2) named locals: yes — each holds a value the following statement consumes.
- (3) `s1`: yes — it carries the `base + 0xC` pointer that the very next
  statement stores into `s.sp1C`, three times over. The target itself keeps this
  value in one register ($s1) across the whole body, so this is the program's own
  variable, not an added one.
- (4) duplicated `s.sp43 = 0x14;`: the STORE is real and required on both paths
  (the callee reads offset 0x43). What has no observable effect is the CHOICE to
  write it twice rather than once at the join — the two spellings are
  byte-identical after cross-jump. That is why it is FAKE-annotated and claimed
  under a sanctioned family below, not defended as neutral.
- (5) `cond`: the read and the test are real and required. What has no observable
  effect is naming it and declaring it one statement earlier. Same treatment:
  FAKE-annotated and claimed under a sanctioned family.

## T2 human-programmer:
- (1)-(3): yes, plainly. A human filling a GPU primitive declares a struct,
  names the intermediates, and reuses one pointer variable for the successive
  table bases. Nothing here would draw a "why is this here?".
- (4): a human writing "each colour case sets the three colour bytes" writes
  exactly this shape, and it is the shape the rest of the file uses; but the
  honest answer is that I chose it to move the scheduler, so it is annotated
  rather than argued as natural.
- (5): a human may or may not name the read; I named it for codegen, so it is
  annotated.

## T3 GCC-internals justification:
Yes — and it is stated openly rather than disguised as program logic, which is
why (4) and (5) are annotated and claimed under frozen families rather than
presented as ordinary C. The mechanism for (4) is the FIRST scheduling pass:
`schedule_select` keeps, inside one equal-`INSN_PRIORITY` group, the insn with
the largest `potential_hazard` (tools/gcc-2.7.2/sched.c:2717).
`potential_hazard` (tools/gcc-2.7.2/sched.c:1327) returns 0 for an insn on no
function unit and a positive value for one whose unit has `max_blockage > 1`;
tools/gcc-2.7.2/insn-attrtab.c:6298 gives the MIPS "memory" unit
`max_blockage 3`, while an `addiu` (attr type "arith") is on no unit. So a ready
store always displaces a ready address-arith insn at equal priority, and the
`best_insn != 0` guard means ready[0] can only be kept, never promoted. Moving
the two `sb` stores into the arms removes them from that pass's basic block 4
(dump: tmp/grind/func_80073200/s9/bb4_sched.txt), so the `(s32)&s` argument set
survives its T-50 step. The mechanism for (5) is LUID order into the same pass.
Constructs (1)-(3) need no GCC-internals story at all — they are explained by
the program's own logic.

## T4 permuter/search provenance:
No construct in this diff came from a search tool. The s4/s5 permuter campaigns
found nothing that survived the honest sandbox (banked as killed in
hypotheses.md), and the s7/s8 enumeration sweeps are all recorded as dead. (4)
was derived this session from the sched.c predicate above — I read the dump and
the pass source, predicted that removing the stores from block 4 would free the
T-50 step, and the first spelling of that prediction measured 0. (5) is
inherited from s8 and was re-ablated this session (removing it regresses 0 → 7,
tmp/grind/func_80073200/s9/a_no_cond.c).

## T5 family check:
- (1)-(3) match no forbidden family: no register pin, no `__asm__`, no
  scheduling barrier, no volatile of any spelling, no dead local, no dead store,
  no self-assign, no alias rename, no cast pun, no `if (1)`/`do{}while(0)`
  wrapper, no unused array, no frame coercion, no `(void)&x`. Every one of them
  is a live value-carrying declaration.
- (4) is the duplicated-statement-into-arms family, claimed below. It is NOT a
  dead store (the store is real on both paths) and NOT a duplicated READ.
- (5) is the named-intermediate family, claimed below. It is a FRESH local, not
  a borrow, so it is not the variable-reuse / staged-value family.
- Nothing here is a first reach of an uncovered family.

## T6 naming-announces-intent:
No name in the diff is `pad`, `dummy`, `unused`, `spill`, `tmp_pad`, `slack`,
`_buf` or any sibling. `tmp`, `s1`, `v1`, `idx`, `cond`, `ctx`, `base1`,
`base2` are all value-descriptive or role-descriptive, and every one of them is
read. `var_v0` (the s8 colour-staging local) and `v12` (the s3 0x12 constant
holder) are both GONE from this body — `v12` was ablated this session and proved
no longer load-bearing (score 0, 203 insns without it).

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: duplicated statement into arms
  SCOPE: "Writing the SAME real statement in two or more control-flow arms — instead of sharing one copy via a label/goto — is a legitimate matching technique, **including** when:"
  PRECEDENT: .claude/rules/duplicated-statement-into-arms.md:63
    (prereq 1 REAL: the sp43 store is needed on both paths — the callee reads
     offset 0x43; prereq 2 BYTE-NEUTRAL: build_insns 203 == target_insns 203 and
     `--diff` shows 0 source-level / 0 operand-only hunks, i.e. find_cross_jump
     re-merged the copies; prereq 3 LEVER-EXHAUSTION: hypotheses.md s4-s8 — two
     permuter campaigns, the 1957-ordering spelling_enum sweep, all 23 orderings
     of this call group, the addr-local naming, every one measured >= 2, and
     this session's sched.c:2717 reading explains WHY that whole axis is
     unreachable by reordering; prereq 4 ANNOTATION: present, quoted below;
     prereq 5: layer-1 + layer-2 review is the driver's to run.)

  FAMILY: named-intermediate declaration order
  SCOPE: "A fresh named intermediate therefore qualifies under this entry **whatever GCC pass it acts through** (LUID bias, cse.c re-materialization, allocno priority), provided ALL of: (1) once-written"
  PRECEDENT: .claude/rules/no-new-park-categories.md:204
    (prong 1 once-written: `cond` is assigned exactly once and read once;
     prong 2 real value: the `lh %gp_rel(D_800A3580)` read is in the target's own
     bytes at asm/funcs/func_80073200.s:151-157, the naming only relocates it;
     prong 3 byte-neutral: build_insns 203 == target_insns 203;
     prong 4 fresh, not a borrow; prong 5 destination not live-pre-initialized;
     prong 6 dump-proven mechanism + documented exhaustion + FAKE annotation, all
     present.)

ANNOTATION-CONFORMANCE:
  /* FAKE: `s.sp43 = 0x14;` is written in BOTH arms instead of once
   * at the join.  Byte-neutral - jump2's find_cross_jump re-merges
   * the two identical arm tails, so nothing extra materializes
   * (build_insns 203 == target_insns 203).  mechanism: the FIRST
   * scheduling pass, schedule_select's `potential_hazard` ready-list
   * swap (tools/gcc-2.7.2/sched.c:2717).  Keeping the two `sb` stores
   * out of that pass's basic block 4 removes the only ready insns
   * that could displace the `(s32)&s` argument set at its T-50 step.
   * lever-exhaustion: memory/grind/func_80073200/hypotheses.md s4-s8
   * (two permuter campaigns, the 1957-ordering spelling_enum sweep,
   * all 23 orderings of this call group, the addr-local naming). */

  /* FAKE: `cond` names the D_800A3580 read as a fresh once-written,
   * once-read intermediate declared ahead of the pointer bump, so the read
   * is emitted before the branch rather than after it.  mechanism: LUID
   * order into the first scheduling pass - the named read becomes the
   * delay-slot-fillable insn the target puts between `lw v0,0x18(s0)` and
   * `beqz` (asm/funcs/func_80073200.s:151-157).  lever-exhaustion:
   * memory/grind/func_80073200/hypotheses.md s5/s6 (the complementary
   * in-block form and both declaration-order sweeps measured dead); ablated
   * again THIS session - removing it regresses 0 -> 7
   * (tmp/grind/func_80073200/s9/a_no_cond.c). */

Both annotations carry all three required parts: WHAT the construct is,
a NAMED GCC-pass MECHANISM (sched.c schedule_select / potential_hazard, and
LUID order into the same pass), and a pointer to the LEVER-EXHAUSTION record in
the ledger.
