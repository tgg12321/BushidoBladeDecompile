# Evidence bank — func_80021280

## s1 (recon, 2026-08-03) — floor 2 re-confirmed; diff reduced to one ADJACENT pair

- Baseline: candidate applied to src, `sandbox --disable all` = 2 (72/72), `canonical` = C.
  The entire distance is the position of `move $a1,$zero` in the loop2 preamble:
  target has it FIRST (0x800212F0), before `move $t1,$v1 / li $t4,4 / li $t3,3 /
  li $t2,1 / lui+lh mode / lui+lbu t0`.
- NEW BEST FORM (form C, still floor 2 but structurally tighter): block-scope
  no-init decls + assignments `t1=val; a1=0; t4=4; t3=3; t2=1; mode=..; t0=..;`.
  Emitted order follows source EXACTLY (verified by objdump: t1,a1,t4,t3,t2,mode,t0)
  — NO pass reorders this block. Remaining diff = swap of the two adjacent moves.
- KILL: real-loop spellings (do-while both layouts; WIP's `for` at 34 consistent).
  Loop notes → loop.c hoists the bare constant 5 (`li 5`) into the preamble and
  scrambles RA; target keeps `addiu $v0,$zero,5` INSIDE the loop (0x8002134C).
  Also measured: mode/t0 GLOBAL loads do NOT hoist even with notes (invariant_p
  rejects MEMs vs the sh-through-a2 loop stores; no alias analysis in 2.7.2).
  ⇒ original loop2 compiled WITHOUT loop.c ⇒ goto-shaped loop is the right family.
- SUPERSEDES WIP root_cause: the banked "loop.c biv-init vs move_movables
  relocation" mechanism cannot apply to the in-context build — goto loops carry
  no NOTE_INSN_LOOP_BEG, so loop.c never runs on them. (The WIP standalone repro
  presumably used a real loop.) The a1-placement question is a straight
  scheduling/RA question, not a loop.c one.
- KILL: no-init decls with `a1 = 0;` FIRST (target statement order) → 19.
  Mechanism: a1 live across `t1 = val;` adds an a1<->t1 pseudo conflict which
  flips the allocator low-reg tie → the $a1/$a2 swap returns. Boundary measured
  precisely: a1=0 at position 1 → 19; at position 2 (after t1=val) → 2.
  So the final flip must NOT add that conflict at RA time → points at a
  post-RA reorder (sched2-level) or a t1-set spelling RA tolerates.
- Neutral: splitting `a1 << 2` into `s32 sh2 = a1 << 2;` — floor 2 unchanged,
  identical bytes (WIP avenue 3 measured dead).
- Sibling scan: no near-duplicate leads for func_80021280 (tmp/duplicates.txt,
  tmp/duplicates_leads.txt both negative).
- Open mechanism question for forensics: in the OLD floor-2 form (a1=0 textually
  LAST), emitted order was t1,t4,t3,t2,a1,mode,t0 — some pass lifted a1 past the
  two load pairs but not past the li's; in form C nothing moves at all. Diffing
  `-da` dumps (.sched/.greg/.sched2) of the two forms on a standalone repro will
  identify the pass and its priority rule — that rule is the lever that could
  lift `move a1,zero` past `move t1,v1` post-RA.
- Artifact: tmp/grind/func_80021280/s1/formC_floor2_disasm.txt (form-C objdump).

- WIP rejected_form: shadow-redeclare a1 inside inner block (s32 a1 = 0; shadowing outer) -> new pseudo, no priority boost, floor back to 19

- WIP rejected_form: hoist t1/t4/t3/t2/mode/t0 to function-scope decls-without-initializer + plain assignments (to legalize a1=0 first) -> floor 19 (loses the nrefs-priority merge benefit)

- WIP rejected_form: single nested sub-block { a1=0; { t1..t0 decls; loop } } -> floor 19

- WIP rejected_form: fully inline mode/t0/constants directly in loop body (no named locals at all) -> floor 41, build_insns 75 (GCC does NOT auto-hoist the global reads through the aliasing a2 pointer stores; recomputes per-iteration)

- WIP rejected_form: convert loop2 to `for (a1=0; a1<3; a1++)` with continue instead of goto/label -> floor 34

- WIP rejected_form: reorder t1/t4/t3/t2/mode/t0 declaration permutations (t1 last, mode/t0 first, t2/t3/t4 swapped, etc.) -> floor stays 2 or gets worse (3-6), never better

- WIP rejected_form: redundant reload for t1 (`*(u16*)(a2+0x48)` instead of `= val`) -> floor 23

- WIP rejected_form: widen t1 to s32 -> floor 4

- WIP rejected_form: comma-operator stage `u16 t1 = (a1 = 0, val);` to force a1=0 first -> NOT TRIED (rejected as unnatural/cheat-shaped before testing; avoid)

- == imported from memory/wip notes.md ==
# func_80021280 — WIP TL;DR

**Floor: sandbox distance 19 (HEAD) -> 2 (this candidate).** Not yet closed to 0.

## What closed 19 -> 2
The function has TWO loop counters: the outer `a1` (used across the whole
function body, incl. loop1) and, originally, a fresh `s32 i = 0;` for a second
nested loop (loop2). Replacing `i` with a reuse of the existing `a1` variable
(same identifier, reassigned `a1 = 0;` right before loop2, used throughout
loop2's body in place of `i`) fixed a GLOBAL register-allocation priority
mismatch: originally the pointer variable `a2` and the counter `a1` had their
physical registers SWAPPED relative to target (target wants counter in the
lower-numbered reg, pointer in the higher). Reusing `a1` boosts its combined
reference count (nrefs) enough to flip the allocator's low-register preference
in `a1`'s favor, matching target. This is the SOTN-sanctioned variable-reuse
family — cheat-reviewer PASSed it (see meta.json).

## What's left: a 2-instruction (really ~4-slot) scheduling reorder
Deep-dived via `cc1 -da` RTL dumps on a standalone repro (see meta.json
`root_cause` for the full mechanism). Short version: GCC 2.7.2's `loop.c`
relocates the loop-2 induction variable's zero-init (`a1 = 0;`) to a fixed
point relative to two other relocated instructions (loop-invariant global
reads `mode = D_800A38DC;` and `t0 = D_800A384C;`), via two DIFFERENT
relocation mechanisms that fire at different times during the loop scan.
Result: final order is `t1, t4, t3, t2, a1, mode, t0` — target wants
`a1, t1, t4, t3, t2, mode, t0` (a1 needs to be FIRST, not fifth).

Since `t1/t4/t3/t2/mode/t0` are all declared-with-initializer LOCALS in the
same C block as the `a1 = 0;` reassignment, C89 requires all declarations to
precede all statements in that block — so `a1 = 0;` (a plain reassignment,
not a declaration) can never be moved ahead of them in the source text
without either illegal C or an unnatural coercion (rejected, see meta.json).

**~10 variant reorderings/type changes were tried (see meta.json
`rejected_forms`) — none beat floor 2.** The next agent should NOT re-try
those. Promising untried avenues are listed in meta.json `next_avenues`
(directed permuter seeded from this floor; reading `tools/gcc-2.7.2/loop.c`
directly for the biv/movable relocation call sites; restructuring the very
first loop-body statement that triggers the biv scan).

## Resume instructions
1. Apply `candidate.c` to `src/code6cac.c` (replaces `func_80021280`, currently
   at line ~2414).
2. Confirm floor: `& tools/wteng.ps1 <target> sandbox func_80021280 --disable all`
   should read `"score": 2`.
3. Continue from `next_avenues` in meta.json. Do NOT re-derive `rejected_forms`.
4. On close to 0: `retire func_80021280`, verify SHA1, cheat-reviewer PASS
   (already have one for the current candidate.c form — get a FRESH one if the
   final closing form differs), then delete this WIP dir.


- [s1] Baseline this session: sandbox --disable all = 2 (72/72 insns), canonical = C, on the WIP candidate applied to src/code6cac.c

- [s1] The entire distance-2 is the position of move $a1,$zero in the loop2 preamble: target 0x800212F0 has it FIRST, ours had it 5th (old form) / 2nd (new form C)

- [s1] Target materializes the constant 5 INSIDE the loop (addiu $v0,$zero,5 at 0x8002134C) and loads mode/t0 exactly once in the preamble => original loop2 was compiled WITHOUT loop.c LICM => goto-shaped loop is the correct family; every note-carrying spelling (for/while/do) is dead

- [s1] GCC 2.7.2 loop.c will NOT hoist the mode/t0 global loads even with notes: invariant_p rejects MEMs against the sh-through-pointer loop stores (no alias analysis) - measured directly, confirms WIP floor-41 datum

- [s1] SUPERSEDES WIP root_cause: the loop.c biv-init-relocation mechanism cannot apply to the in-context goto-loop build (no loop notes => loop.c never runs); a1 placement is a scheduling/RA question

- [s1] RA boundary measured: a1=0 textually before t1=val => a1<->t1 conflict => swap returns (19); after t1=val => RA correct (2). The final flip must not add that conflict pre-RA => post-RA reorder (sched2) or a conflict-tolerant t1 spelling

- [s1] NEW BEST FORM banked (form C, candidate.c): no-init decls + t1=val; a1=0; t4=4; t3=3; t2=1; mode=..; t0=..; emitted order exactly textual, diff = one adjacent transposition

- [s1] Open forensics question: which pass lifted a1 past the two load pairs in the OLD form (textual 7th -> emitted 5th) while form C is emitted verbatim - answerable by diffing -da dumps of the two forms on a standalone repro

- [s1] Sibling/duplicate scan negative: func_80021280 absent from tmp/duplicates.txt and tmp/duplicates_leads.txt

## s2 (structural, 2026-08-03/04) — BYTES PROVEN TO 0; acceptance pending an owner ruling on construct scope

- Baseline re-confirmed: form C applied to src -> sandbox --disable all = 2 (72/72).
- NEW STRUCTURAL FACT (from target asm read): target keeps `mode` in $a3 (lui/lh
  $a3 at 0x21304-08) — the same register as loop1's compare temp. Reusing the C
  variable `a3` for mode (declared s32 so `a3 = *(u16*)(a2+0x48)` emits lhu and
  `a3 = D_800A38DC` emits lh) is FLOOR-NEUTRAL: 2 with form-C order, 19 with
  a1-first order. Same sanctioned variable-reuse family as the a1 reuse; more
  faithful to the original but does NOT fix the swap. Kept in the candidate.
- KILL: a1-first + t1 moved after the li group (a1; t4; t3; t2; t1=val; a3; t0)
  -> 21 (the 19 RA regression + 2 ordering). The regression tracks a1's textual
  position, not t1's.
- KILL: a1-first + u8 typings of t4/t3/t2 -> 19. Type class of the li group does
  not participate in the tie.
- MECHANISM NAILED (BB2_ALLOC_DEBUG on instrumented tools/gcc-2.7.2/cc1, whole
  code6cac.c compile, func_80021280 section — tmp/grind/func_80021280/s2/
  allocdbg_a1first.raw): global.c allocno_compare priority
  = floor_log2(nrefs)*nrefs/livelen*10000, tie -> lower pseudo number.
  In the a1-first form: POINTER pseudo 74 nrefs=11 livelen=45 pri=7333 beats
  COUNTER pseudo 73 nrefs=12 livelen=50 pri=7200, so the pointer is allocated
  first and takes $a1 (the swap = the 19). In form C the counter's livelen is 49
  (born one insn later, after move t1,v1) -> pri 7346 > 7333 -> counter wins $a1
  but the preamble order is then wrong by the adjacent swap. This supersedes the
  s1 "a1<->t1 conflict" hypothesis: it is a pure live-LENGTH priority artifact;
  one insn of livelen is the whole margin. nrefs are UNWEIGHTED occurrence
  counts: counter's 12 and pointer's 11 each map 1:1 to target instructions, so
  NO natural 13th counter ref exists.
- Deltas that flip the tie with a1-first: counter nrefs 12->13 (pri 7800) or
  livelen 50->49; pointer nrefs -1 or livelen +1. None reachable naturally.
  Assignment-duplication per [[duplicated-statement-into-arms]] is ARITHMETICALLY
  INSUFFICIENT: every counter-referencing assignment (sh a1,0x88 / sh a1,0x8E)
  also references the pointer; +1 each -> counter 3*13/50=7800 vs pointer
  3*12/45=8000 — the pointer gains more from every paired ref because its live
  range is shorter, at every k. The only counter-pure statements are the loop
  tail (a1++ / a1<3) and the preamble a1=0 (no arms available).
- CLOSED TO 0 (construct pending ruling): duplicate the loop tail
  `a1++; if (a1 < 3) goto loop2_21280; return;` into the `if (a0 == 0)` arm
  (replacing `goto next_21280`), FAKE-annotated. flow counts +3 counter refs
  pre-RA (pri 9000 > 7333, allocation order otherwise unchanged since 9000 <
  the next-higher allocno's 10000); jump2 cross-jump (post-reload) re-merges the
  identical addiu/slti/bnez suffix; the emitted branch is exactly target's
  `beqz a0,.L80021388`. sandbox --disable all = 0, 72/72. Full disasm:
  tmp/grind/func_80021280/s2/final_zero_disasm.txt (byte-exact incl. the
  lhu/nop/sh load-delay nop and single shared tail).
- KILL (placement matters): the same tail duplicated into the store5
  FALL-THROUGH arm -> 2 with build 73: sched1 hoists the duplicate's addiu into
  the lhu 0x26C load-delay slot, breaking suffix identity, so cross-jump merges
  only slti/bnez and leaves addiu + an inserted j. The arm must contain no loads
  for the full-suffix merge. Banked: rejected/tail-dup-store5-arm-sched1-hoist.c.
- Layer-1 cheat-reviewer verdict on the 0-form: FAIL — (a) s2 evidence was not
  yet persisted at review time (fixed by this entry), and (b) a REAL scope
  question: [[duplicated-statement-into-arms]]'s SOTN evidence base is
  single ASSIGNMENT statements; duplicating a loop-continuation tail
  (control-transfer: backward goto + return) is an extension the reviewer says
  needs its own owner ruling. Hence s2 outcome = ruling-request, floor reported
  2 (last uncontested), bytes-proven-0 form kept in src + candidate.c.
- Fallback if the ruling refuses: form C (floor 2, no FAKE constructs) is the
  s1 candidate.c in git history; the only other known lever family for the
  final swap is the s1 frontier's post-RA-scheduling forensics line.

## s2-post (operator, 2026-08-04 ~02:30) — ACCEPTANCE CONFLICT, owner-gated

The 00:11 Judge construct-scope ruling PASSed the loop-tail duplication
(docs/grind/decisions.md 3485). The operator then ran the completion path:
candidate applied to src, sandbox --disable all = 0 (72/72), retire dropped
the $5<->$6 rule, full clean build SHA1 == oracle. The MANDATORY layer-2
fresh cheat-reviewer then returned **FAIL** (default-FAIL, fresh agent):
- test-5: [[duplicated-statement-into-arms]]'s SOTN base is assignment/store
  duplication; a loop-continuation control structure (increment + backward
  conditional branch + implicit return) into an unrelated arm is an
  UN-PRECEDENTED extension needing its own SOTN evidence; the Judge's
  mixed-exit-forms bridge is the "different spelling of the same intent"
  loophole no-new-park-categories forbids.
- test-2: `if (a0==0) goto next_21280;` is what a real `continue` compiles
  to; no 1998 programmer re-types the loop tail inline into one arm.
- test-3: the load-bearing justification is global.c allocno_compare
  priority arithmetic — a coercion signature.
Per review-discipline-before-commit (no override of layer-2 FAIL; no
self-resolved conflicts), the operator reverted src+regfix to HEAD (rule
restored, oracle green) and parked the function OWNER-PENDING. The
bytes-proven 0-form remains candidate.c; the factual crux for the owner is
recorded in decisions.md (OWNER-PENDING entry, 2026-08-04). Nothing else in
the 0-form was flagged (preamble reorder, a3 s32 widening, decl order all
cleared by the reviewer).
