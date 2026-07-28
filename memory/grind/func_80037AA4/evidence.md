# Evidence bank — func_80037AA4

- WIP rejected_form: no-dummy pin-free (16; absent 8-byte frame costs 2 sp-adjust insns)

- WIP rejected_form: decl reorder var_a0 first (16; swap+frame unchanged)

- == imported from memory/wip notes.md ==
# func_80037AA4 — WIP/BLOCKED (sum-and-scale over D_80102810 entries)

## TL;DR
HEAD "matches" via THREE `register asm()` pins (a2/a0/v0) PLUS an unused
`s32 sp_dummy[2]` frame-padding array. Two coupled blockers prevent a clean
pure-C close:

1. **8-byte unused stack frame.** The target has `addiu sp,sp,-8` / `+8` with
   NO sw/lw to sp (no spill, nothing stored). A leaf function with no
   used locals gets NO frame in GCC 2.7.2, so the original had a real stack
   local whose content is UNRECOVERABLE from the pure logic. The only way to
   reproduce the frame is a forbidden coercion (`sp_dummy[2]` unused array /
   address-taken / volatile) -- the dead-vars-local-array cheat
   ([[register-alloc-pure-c]] Lever D, FORBIDDEN). candidate.c (no dummy)
   produces NO frame -> -2 insns vs target.
2. **a0<->v1 register rename.** Target: accumulator var_a0 -> a0, pointer
   var_v1 -> v1. Pin-free GCC swaps them (accumulator->v1, pointer->a0). Same
   tied-priority-rename class as func_80037A20 / func_80044098.

## Measured
- HEAD honest distance (pins+dummy stripped): 14.
- candidate.c (no pins, NO dummy): 16 diffs -- WORSE, because the absent frame
  costs the 2 sp-adjust insns the dummy was supplying. Floor NOT lowered.
- decl reorder (v2): 16, swap+frame unchanged.

## Why blocked (policy, not just difficulty)
The 8-byte frame has no non-cheat pure-C reconstruction: the original stack
local's type/content is unknown, and any unused-array/address-of/volatile form
that forces the frame is a forbidden coercion. This needs either (a) recovery of
what the real stack local was (data-flow / sibling-function evidence), or (b) a
user policy call on reconstructing an unused frame. PLUS the a0<->v1 rename.

## Avenues for next session
- Investigate whether a sibling/caller reveals the original 8-byte local's
  purpose (then it's a legitimate named local, not a cheat).
- If the frame is genuinely an artifact with no real local, escalate as a
  policy question (reconstruct-unused-frame) -- not worker-closable.
- decomp-permuter for the a0<->v1 rename once the frame question is resolved.

## Floor
- HEAD: 14 (3 pins + sp_dummy[2] frame cheat). candidate.c: 16 (cheat-free, but
  short the frame). Not lowered; blocked on the frame-reconstruction policy +
  tied register rename.


## s1 (recon, 2026-07-28) — floor 14 -> 4; both WIP "blockers" dissolved

- [s1] **FLOOR 14 -> 4 with one pure-C edit.** The a0<->v1 rename is a plain
  decl-order lever: declare the POINTER first (`s8 *var_v1;` before the s32
  decls) in the guarded do-while form. WIP's killed probe had moved var_a0
  first (inert); var_v1-first was never measured. sandbox --disable all = 4
  with zero pins, zero dummy, edits in src. Remaining 4 = ONLY the missing
  8-byte frame (build 20 insns vs target 23).
- [s1] **The 8-byte frame is NOT a cheat-only construct** (WIP claim FALSE, per
  [[phantom-frame-slots-gcc272]]). Measured trigger for THIS function: any
  loop whose trip count is not provably >=1 (unguarded entry-test while/for)
  gets a phantom stack temp -> cc1 `vars= 8`, zero stores, zero dead decls.
  Guarded forms (if(n>0)+do-while / +while) always give vars=0. Instrument:
  cc1 .frame comment (frameprobe.sh / sweep*.py in tmp/grind/func_80037AA4/s1/).
- [s1] **Unguarded indexed form reproduces ALL target structure**: `for (i=0;
  i<n; i++) sum += ((CamEnt*)&D_80102810)[i].unk18;` with 0x28-size struct ->
  strength-reduced walking pointer, la AFTER blez (giv init in preheader
  between duplicated exit test and loop), lw 24(p), addu p,p,40, vars=8,
  21 cc1 insns == target. Byte-arith spelling `+ i*0x28 + 0x18` folds the
  +24 into the la (giv base D_80102810+24, lw 0(p)) — struct field spelling
  is required for the unfolded base.
- [s1] **Target tail is provably the T0 spelling** (`v0=sum; if(sum<0)
  v0=sum+0x1FFF; sum=v0>>13; return 0xF-sum;`): move $2,$4 in bgez delay slot,
  addu $2,$4,8191 (reads sum), sra $4,$2,13 (quotient coalesced into sum's
  pseudo), subu $2,$2,$4.
- [s1] **The one remaining coupling**: on the unguarded indexed base, T0's 4
  tail refs on sum make global-alloc allocate sum before the strength-
  reduction giv -> sum=$3/p=$4 (swapped). Decl order is INERT there (pointer
  is a compiler giv, not a user var — all 24 perms identical). Measured-inert
  flip levers: init orders, while/for/postinc spellings, plain-add, u8-arith,
  fresh-quotient var (coalesces the tail move away), quotient-into-i. The only
  measured flip (in-place tail, sum refs 2) breaks the tail (v0 -> $3, standalone
  move, in-place adds; score 6). Score map: guarded+pfirst=4, indexed+inplace=6,
  indexed+T0=11, old pinned honest=14, clean-unflipped=16.
- [s1] Division spelling `0xF - sum/0x2000` (semantically identical) is
  measured WORSE: perturbs loop schedule (i++ hoists above lw -> nop) and
  emits the in-place division tail. See rejected/division-tail-breaks-loop-sched.c.
- [s1] No sibling/duplicate exists (tmp/duplicates.txt full pairwise list: no
  80037AA4 entry; neighbors func_80037A20/func_80037B00 are themselves
  pin-carrying INCOMPLETE).

- [s1] Floor 4 in src NOW: guarded do-while pointer-walk with s8 *var_v1 declared first; residual = missing addiu sp,-8/+8 only (build 20 vs target 23 insns)

- [s1] cc1 .frame vars= is a direct frame gradient: pinned+dummy form vars=8 regs exact; clean guarded forms vars=0

- [s1] vars=8 trigger law (this function): loop trip count not provably >=1 => phantom stack temp, zero stores; if(n>0) guard kills it

- [s1] Unguarded for + 0x28-struct indexing strength-reduces to the exact target loop (la after blez, lw 24(p), addu p,p,40, 21 insns, vars=8); byte-arith spelling folds +24 into la (wrong)

- [s1] Target tail == T0 spelling: move $2,$4 in bgez delay, addu $2,$4,8191, sra $4,$2,13, subu $2,$2,$4

- [s1] Score map: guarded+pfirst=4, indexed+inplace=6, indexed+T0=11, old pinned honest=14, clean unflipped=16

- [s1] No duplicate/sibling lead (tmp/duplicates.txt has no 80037AA4 pair; both neighbors are pin-carrying INCOMPLETE)

## s2 (structural, 2026-07-28) — floor stays 4; vars=8 trigger FOUND naturally, flip arithmetic fully characterized

- [s2] **Natural guarded vars=8 trigger exists**: `if (var_a1 < var_a2)` (a1 just
  zeroed) instead of `if (var_a2 > 0)`. Expand emits a reg-reg slt pseudo; cse
  folds the operand (a1=0 known); combine folds slt+branch -> blez a2, orphaning
  the slt pseudo. gdb-proven mechanism: reload1.c alter_reg(orphan, -1) from
  reload() allocates a 4-byte spill slot for the combine-orphaned pseudo
  (stale refs, renumber<0, class ST_REGS) -> vars=8, ZERO stack stores, and the
  slot never appears in ANY -da dump (allocated at reload, RTX discarded).
  Refines the s1 trip-count law: the real trigger is an ORPHANED COMPARE PSEUDO,
  which unguarded entry-test duplication produces as a special case.
- [s2] g1 form (orphan guard) sandbox = 11 with build 23/23 == target insn
  count: sp,-8/+8 and final nop all correct; ONLY diff = sum<->p swapped
  ($3/$4) in every sum/p insn. rejected/orphan-guard-sum-p-swap.c.
- [s2] **The swap is exact allocno arithmetic** (global.c allocno_compare:
  pri = floor_log2(refs)*refs/live_length*10000; tie -> lower pseudo number).
  Flow refs are loop-depth-weighted (x2 inside the loop). Byte-locked counts:
  sum = 1(init)+4(loop addu, weighted)+5(T0 tail)+1(guard slt charge) = 11 refs
  / len 15 -> 22000; pointer = 1(la)+2(lw)+4(latch addu) = 7 refs / len 7 ->
  20000. Sum first -> sum=$3 (WRONG). g0 (zero-compare guard, no slt charge):
  sum 10/15 = 20000 ties p 20000, tie-break lower allocno = POINTER (declared
  first) -> p=$3/sum=$4 (target). This is WHY pointer-decl-first works on g0
  and why decl order is INERT on every orphan form (no tie to break).
- [s2] **cse.c make_regs_eqv canonicalizes the zero-equivalence class to its
  LONGEST-LIVED member** — sum, always (its last use is the final subu). Every
  guard spelling (a1/a0/v0/reversed/comma) charges the +1 ref to sum. Un-steerable
  unless the tail's last sum use moves earlier, which the T0 tail forbids
  (quotient must write sum's pseudo for sra $4/subu bytes).
- [s2] Measured-dead flip levers on the g1 base: 120/120 decl orders; init
  orders; guard operand identity; loop stmt order; fused accumulate (22 insns);
  address-temp split tp=p+0x18 (cse folds into MEM before flow, tp never exists
  at flow); split-addiu chains 0x1000+0xFFF / x3 (cse folds, len unchanged);
  nested same-expr double guard (cse dedups fully); a1+1<=a2 / a2>=1 second
  guards (both slts fold -> TWO blez in bytes; jump2 does NOT dedup identical
  branches, n=22); && spellings (same); a2>=1 && a1<a2 reversed (second slt
  lands in bb1 where combine has no LOG_LINKS to the zero def -> slt/beq survive
  in bytes, n=20, vars=0 — but its sum len 17 -> 19411 CONFIRMS the flip
  threshold arithmetic empirically: that variant allocates sum=$4/p=$3).
- [s2] Flip win condition (proven, quantitative): sum pri must be <= 20000,
  i.e. with byte-locked refs 11: sum live_length >= 17 (+2 flow-time insns
  inside sum's range, outside p's loop range, deleted between flow and final).
  Only combine deletes insns in that window, and only within a basic block via
  LOG_LINKS; the only known construct is the orphan-slt chain itself (+0 len
  net in bb0). All plain-arithmetic extra-insn spellings are cse-folded or
  flow-dead-store-deleted BEFORE the count.
- [s2] Route B (unguarded indexed struct form, T0 tail) has IDENTICAL numbers:
  sum 11/15 = 22000 vs giv-pointer 7/7 = 20000 (giv pseudo number high, no tie
  anyway). Routes A and B are the same locked arithmetic; s1's "indexed+T0=11"
  and s2's "orphan-guard=11" are the same 11-insn register swap.
- [s2] Floor-4 g0 form re-verified in src at session start and end (sandbox 4).

- [s2] phantom-frame mechanism refined: reload1.c alter_reg allocates spill slots for combine-orphaned compare pseudos (stale refs, renumber<0); the s1 trip-count law is the unguarded special case of this

- [s2] flow REG_N_REFS are loop-depth-weighted (x2 per loop level); byte-locked counts for this function: sum 11 refs/len 15, pointer 7/7

- [s2] g0 floor-4 works precisely because sum 10/15 == p 7/7 == 20000 tie, broken by pointer's lower pseudo number (decl-first)

- [s2] flip win condition quantified: sum live_length >= 17 (or -1 sum ref, impossible: refs byte-locked + cse longest-lived canonicalization) via flow-surviving combine-deleted bb0 insns; only branch-fed chains survive cse and combine only folds within a bb

- [s2] route B (unguarded indexed, T0 tail) has IDENTICAL arithmetic (sum 11/15 vs giv 7/7): s1's indexed+T0=11 and s2's orphan-guard=11 are the same register swap; both routes converge on the same lock

- [s2] floor-4 g0 form re-verified in src at session start and end (sandbox 4, build 20 vs target 23)
